#include "DashboardBridge.hpp"

#include "types/Frame.hpp"

#include <QTimer>
#include <chrono>

namespace csx::ui {

namespace {

template <typename Signal>
void emitIfChanged(QString& current, const QString& next, DashboardBridge* owner, Signal signal) {
    if (current == next) {
        return;
    }
    current = next;
    emit(owner->*signal)();
}

constexpr int kHistoryCapacity = 72;
constexpr auto kHistoryPushInterval = std::chrono::milliseconds(100);
constexpr auto kTextPushInterval = std::chrono::milliseconds(250);

void appendSample(QVariantList& history, const double value) {
    history.append(value);
    while (history.size() > kHistoryCapacity) {
        history.removeFirst();
    }
}

bool shouldPushText(const std::chrono::steady_clock::time_point now,
                    std::chrono::steady_clock::time_point& lastPush) {
    if (now - lastPush >= kTextPushInterval) {
        lastPush = now;
        return true;
    }
    return false;
}

}  // namespace

DashboardBridge::DashboardBridge(QObject* parent)
    : QObject(parent)
    , startTime_(std::chrono::steady_clock::now())
    , lastHistoryPush_(startTime_)
    , lastThreatReasonPush_(startTime_)
    , lastTelemetryPush_(startTime_)
    , lastDossierPush_(startTime_)
    , cameraStatus_(QStringLiteral("INITIALIZING"))
    , faceStatus_(QStringLiteral("STANDBY"))
    , recognitionEngine_(QStringLiteral("OFFLINE"))
    , targetManifest_(QStringLiteral("NO CONTACTS"))
{
    for (int i = 0; i < 8; ++i) {
        sectorActivity_.append(0);
    }

    auto* uptimeTimer = new QTimer(this);
    uptimeTimer->setInterval(1000);
    connect(uptimeTimer, &QTimer::timeout, this, &DashboardBridge::uptimeChanged);
    uptimeTimer->start();

    auto* textFlushTimer = new QTimer(this);
    textFlushTimer->setInterval(400);
    connect(textFlushTimer, &QTimer::timeout, this, &DashboardBridge::flushPendingText);
    textFlushTimer->start();
}

DashboardBridge::~DashboardBridge() = default;

void DashboardBridge::updateHealth(const csx::core::SystemHealth& health) {
    const double fps = health.fps;
    const double cpuPercent = health.cpuPercent;
    const double ramPercent = health.ramPercent;
    const double frameTimeMs = health.frameTimeMs;

    if (fps_.load(std::memory_order_relaxed) != fps) {
        fps_.store(fps, std::memory_order_relaxed);
        emit fpsChanged();
    }
    if (cpuPercent_.load(std::memory_order_relaxed) != cpuPercent) {
        cpuPercent_.store(cpuPercent, std::memory_order_relaxed);
        emit cpuPercentChanged();
    }
    if (ramPercent_.load(std::memory_order_relaxed) != ramPercent) {
        ramPercent_.store(ramPercent, std::memory_order_relaxed);
        emit ramPercentChanged();
    }
    if (frameTimeMs_.load(std::memory_order_relaxed) != frameTimeMs) {
        frameTimeMs_.store(frameTimeMs, std::memory_order_relaxed);
        emit frameTimeMsChanged();
    }
}

void DashboardBridge::updateThreat(const csx::core::ThreatAssessment& assessment) {
    const int level = static_cast<int>(assessment.level);
    const double score = assessment.threatScore;
    const int defcon = static_cast<int>(assessment.defcon);
    const QString reason = QString::fromStdString(assessment.decisionReason);

    if (threatLevel_.load(std::memory_order_relaxed) != level) {
        threatLevel_.store(level, std::memory_order_relaxed);
        emit threatLevelChanged();
    }
    if (threatScore_.load(std::memory_order_relaxed) != score) {
        threatScore_.store(score, std::memory_order_relaxed);
        emit threatScoreChanged();
    }
    if (defconLevel_.load(std::memory_order_relaxed) != defcon) {
        defconLevel_.store(defcon, std::memory_order_relaxed);
        emit defconLevelChanged();
    }

    {
        std::lock_guard<std::mutex> lock(uiMutex_);
        if (threatReason_ != reason) {
            threatReason_ = reason;
            pendingTextDirty_ = true;
        }
    }
}

void DashboardBridge::setRecording(bool recording) {
    const bool current = recording_.load(std::memory_order_relaxed);
    if (current == recording) {
        return;
    }
    recording_.store(recording, std::memory_order_relaxed);
    emit recordingChanged();
}

void DashboardBridge::notifyFrameUpdated() {
    frameId_.fetch_add(1, std::memory_order_relaxed);
    emit frameIdChanged();
}

void DashboardBridge::updateVisionTelemetry(const int faceCount, const int trackCount,
                                            const QString& cameraStatus, const QString& faceStatus,
                                            const QString& recognitionEngine,
                                            const QString& targetManifest, const double neuralLoad) {
    if (faceCount_.load(std::memory_order_relaxed) != faceCount) {
        faceCount_.store(faceCount, std::memory_order_relaxed);
        emit faceCountChanged();
    }
    if (trackCount_.load(std::memory_order_relaxed) != trackCount) {
        trackCount_.store(trackCount, std::memory_order_relaxed);
        emit trackCountChanged();
    }
    if (neuralLoad_.load(std::memory_order_relaxed) != neuralLoad) {
        neuralLoad_.store(neuralLoad, std::memory_order_relaxed);
        emit neuralLoadChanged();
    }

    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(uiMutex_);
        if (cameraStatus_ != cameraStatus) {
            cameraStatus_ = cameraStatus;
            changed = true;
        }
        if (faceStatus_ != faceStatus) {
            faceStatus_ = faceStatus;
            changed = true;
        }
        if (recognitionEngine_ != recognitionEngine) {
            recognitionEngine_ = recognitionEngine;
            changed = true;
        }
        if (targetManifest_ != targetManifest) {
            targetManifest_ = targetManifest;
            changed = true;
        }
        if (changed) {
            pendingTextDirty_ = true;
        }
    }
}

void DashboardBridge::setBootComplete(const bool complete) {
    const bool current = bootComplete_.load(std::memory_order_relaxed);
    if (current == complete) {
        return;
    }
    bootComplete_.store(complete, std::memory_order_relaxed);
    emit bootCompleteChanged();
}

QString DashboardBridge::threatReason() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return threatReason_;
}

QString DashboardBridge::cameraStatus() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return cameraStatus_;
}

QString DashboardBridge::faceStatus() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return faceStatus_;
}

QString DashboardBridge::recognitionEngine() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return recognitionEngine_;
}

QString DashboardBridge::targetManifest() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return targetManifest_;
}

void DashboardBridge::updateIdentityDossier(const QString& dossier, const QString& status,
                                              const int confidence, const int authorizedCount) {
    if (identityConfidence_.load(std::memory_order_relaxed) != confidence) {
        identityConfidence_.store(confidence, std::memory_order_relaxed);
        emit identityConfidenceChanged();
    }
    if (authorizedCount_.load(std::memory_order_relaxed) != authorizedCount) {
        authorizedCount_.store(authorizedCount, std::memory_order_relaxed);
        emit authorizedCountChanged();
    }

    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(uiMutex_);
        if (identityDossier_ != dossier) {
            identityDossier_ = dossier;
            changed = true;
        }
        if (identityStatus_ != status) {
            identityStatus_ = status;
            changed = true;
        }
        if (changed) {
            pendingTextDirty_ = true;
        }
    }
}

QString DashboardBridge::identityDossier() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return identityDossier_;
}

QString DashboardBridge::identityStatus() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return identityStatus_;
}

void DashboardBridge::updateAnalytics(const QString& lockPhase, const int lockStrength,
                                      const int radarBlips, const double threatScore,
                                      const double neuralLoad,
                                      const std::vector<int>& sectorActivity) {
    if (lockStrength_.load(std::memory_order_relaxed) != lockStrength) {
        lockStrength_.store(lockStrength, std::memory_order_relaxed);
        emit lockStrengthChanged();
    }
    if (radarBlipCount_.load(std::memory_order_relaxed) != radarBlips) {
        radarBlipCount_.store(radarBlips, std::memory_order_relaxed);
        emit radarBlipCountChanged();
    }

    bool lockPhaseDirty = false;
    bool historyChanged = false;
    bool sectorsChanged = false;
    const auto now = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(uiMutex_);
        if (lockPhase_ != lockPhase) {
            lockPhase_ = lockPhase;
            lockPhaseDirty = true;
        }

        if (now - lastHistoryPush_ >= kHistoryPushInterval) {
            appendSample(threatHistory_, threatScore);
            appendSample(neuralHistory_, neuralLoad);
            lastHistoryPush_ = now;
            historyChanged = true;
        }

        QVariantList nextSectors;
        nextSectors.reserve(8);
        for (int i = 0; i < 8; ++i) {
            const int value =
                i < static_cast<int>(sectorActivity.size()) ? sectorActivity[static_cast<std::size_t>(i)]
                                                            : 0;
            nextSectors.append(value);
        }
        if (sectorActivity_ != nextSectors) {
            sectorActivity_ = nextSectors;
            sectorsChanged = true;
        }
    }

    if (lockPhaseDirty) {
        emit lockPhaseChanged();
    }
    if (historyChanged) {
        emit threatHistoryChanged();
        emit neuralHistoryChanged();
    }
    if (sectorsChanged) {
        emit sectorActivityChanged();
    }
}

void DashboardBridge::flushPendingText() {
    bool dirty = false;
    {
        std::lock_guard<std::mutex> lock(uiMutex_);
        if (!pendingTextDirty_) {
            return;
        }
        pendingTextDirty_ = false;
        dirty = true;
    }

    if (!dirty) {
        return;
    }

    emit cameraStatusChanged();
    emit faceStatusChanged();
    emit recognitionEngineChanged();
    emit targetManifestChanged();
    emit identityDossierChanged();
    emit identityStatusChanged();
    emit threatReasonChanged();
}

QString DashboardBridge::lockPhase() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return lockPhase_.isEmpty() ? QStringLiteral("SCANNING") : lockPhase_;
}

QVariantList DashboardBridge::threatHistory() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return threatHistory_;
}

QVariantList DashboardBridge::neuralHistory() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return neuralHistory_;
}

QVariantList DashboardBridge::sectorActivity() const {
    std::lock_guard<std::mutex> lock(uiMutex_);
    return sectorActivity_;
}

QString DashboardBridge::uptime() const {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime_).count();

    const int hours = static_cast<int>(elapsed / 3600);
    const int minutes = static_cast<int>((elapsed % 3600) / 60);
    const int seconds = static_cast<int>(elapsed % 60);

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

}  // namespace csx::ui
