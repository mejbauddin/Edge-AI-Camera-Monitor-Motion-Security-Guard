#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

namespace csx::core {
struct SystemHealth;
struct ThreatAssessment;
}  // namespace csx::core

namespace csx::ui {

class DashboardBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(double fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(double cpuPercent READ cpuPercent NOTIFY cpuPercentChanged)
    Q_PROPERTY(double ramPercent READ ramPercent NOTIFY ramPercentChanged)
    Q_PROPERTY(double frameTimeMs READ frameTimeMs NOTIFY frameTimeMsChanged)
    Q_PROPERTY(int threatLevel READ threatLevel NOTIFY threatLevelChanged)
    Q_PROPERTY(double threatScore READ threatScore NOTIFY threatScoreChanged)
    Q_PROPERTY(QString threatReason READ threatReason NOTIFY threatReasonChanged)
    Q_PROPERTY(int defconLevel READ defconLevel NOTIFY defconLevelChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(QString uptime READ uptime NOTIFY uptimeChanged)
    Q_PROPERTY(int frameId READ frameId NOTIFY frameIdChanged)
    Q_PROPERTY(int faceCount READ faceCount NOTIFY faceCountChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY trackCountChanged)
    Q_PROPERTY(QString cameraStatus READ cameraStatus NOTIFY cameraStatusChanged)
    Q_PROPERTY(QString faceStatus READ faceStatus NOTIFY faceStatusChanged)
    Q_PROPERTY(QString recognitionEngine READ recognitionEngine NOTIFY recognitionEngineChanged)
    Q_PROPERTY(QString targetManifest READ targetManifest NOTIFY targetManifestChanged)
    Q_PROPERTY(bool bootComplete READ bootComplete NOTIFY bootCompleteChanged)
    Q_PROPERTY(double neuralLoad READ neuralLoad NOTIFY neuralLoadChanged)
    Q_PROPERTY(QString identityDossier READ identityDossier NOTIFY identityDossierChanged)
    Q_PROPERTY(QString identityStatus READ identityStatus NOTIFY identityStatusChanged)
    Q_PROPERTY(int identityConfidence READ identityConfidence NOTIFY identityConfidenceChanged)
    Q_PROPERTY(int authorizedCount READ authorizedCount NOTIFY authorizedCountChanged)
    Q_PROPERTY(QString lockPhase READ lockPhase NOTIFY lockPhaseChanged)
    Q_PROPERTY(int lockStrength READ lockStrength NOTIFY lockStrengthChanged)
    Q_PROPERTY(int radarBlipCount READ radarBlipCount NOTIFY radarBlipCountChanged)
    Q_PROPERTY(QVariantList threatHistory READ threatHistory NOTIFY threatHistoryChanged)
    Q_PROPERTY(QVariantList neuralHistory READ neuralHistory NOTIFY neuralHistoryChanged)
    Q_PROPERTY(QVariantList sectorActivity READ sectorActivity NOTIFY sectorActivityChanged)

public:
    explicit DashboardBridge(QObject* parent = nullptr);
    ~DashboardBridge() override;

    void updateHealth(const csx::core::SystemHealth& health);
    void updateThreat(const csx::core::ThreatAssessment& assessment);
    void setRecording(bool recording);
    void notifyFrameUpdated();
    void updateVisionTelemetry(int faceCount, int trackCount, const QString& cameraStatus,
                               const QString& faceStatus, const QString& recognitionEngine,
                               const QString& targetManifest, double neuralLoad);
    void updateIdentityDossier(const QString& dossier, const QString& status, int confidence,
                               int authorizedCount);
    void updateAnalytics(const QString& lockPhase, int lockStrength, int radarBlips,
                         double threatScore, double neuralLoad,
                         const std::vector<int>& sectorActivity = {});
    void setBootComplete(bool complete);

    double fps() const { return fps_.load(std::memory_order_relaxed); }
    double cpuPercent() const { return cpuPercent_.load(std::memory_order_relaxed); }
    double ramPercent() const { return ramPercent_.load(std::memory_order_relaxed); }
    double frameTimeMs() const { return frameTimeMs_.load(std::memory_order_relaxed); }
    int threatLevel() const { return threatLevel_.load(std::memory_order_relaxed); }
    double threatScore() const { return threatScore_.load(std::memory_order_relaxed); }
    QString threatReason() const;
    int defconLevel() const { return defconLevel_.load(std::memory_order_relaxed); }
    bool recording() const { return recording_.load(std::memory_order_relaxed); }
    QString uptime() const;
    int frameId() const { return frameId_.load(std::memory_order_relaxed); }
    int faceCount() const { return faceCount_.load(std::memory_order_relaxed); }
    int trackCount() const { return trackCount_.load(std::memory_order_relaxed); }
    QString cameraStatus() const;
    QString faceStatus() const;
    QString recognitionEngine() const;
    QString targetManifest() const;
    bool bootComplete() const { return bootComplete_.load(std::memory_order_relaxed); }
    double neuralLoad() const { return neuralLoad_.load(std::memory_order_relaxed); }
    QString identityDossier() const;
    QString identityStatus() const;
    int identityConfidence() const { return identityConfidence_.load(std::memory_order_relaxed); }
    int authorizedCount() const { return authorizedCount_.load(std::memory_order_relaxed); }
    QString lockPhase() const;
    int lockStrength() const { return lockStrength_.load(std::memory_order_relaxed); }
    int radarBlipCount() const { return radarBlipCount_.load(std::memory_order_relaxed); }
    QVariantList threatHistory() const;
    QVariantList neuralHistory() const;
    QVariantList sectorActivity() const;

signals:
    void fpsChanged();
    void cpuPercentChanged();
    void ramPercentChanged();
    void frameTimeMsChanged();
    void threatLevelChanged();
    void threatScoreChanged();
    void threatReasonChanged();
    void defconLevelChanged();
    void recordingChanged();
    void uptimeChanged();
    void frameIdChanged();
    void faceCountChanged();
    void trackCountChanged();
    void cameraStatusChanged();
    void faceStatusChanged();
    void recognitionEngineChanged();
    void targetManifestChanged();
    void bootCompleteChanged();
    void neuralLoadChanged();
    void identityDossierChanged();
    void identityStatusChanged();
    void identityConfidenceChanged();
    void authorizedCountChanged();
    void lockPhaseChanged();
    void lockStrengthChanged();
    void radarBlipCountChanged();
    void threatHistoryChanged();
    void neuralHistoryChanged();
    void sectorActivityChanged();

private slots:
    void flushPendingText();

private:
    mutable std::mutex uiMutex_;

    std::atomic<double> fps_{0.0};
    std::atomic<double> cpuPercent_{0.0};
    std::atomic<double> ramPercent_{0.0};
    std::atomic<double> frameTimeMs_{0.0};
    std::atomic<int> threatLevel_{0};
    std::atomic<double> threatScore_{0.0};
    std::atomic<int> defconLevel_{5};
    std::atomic<bool> recording_{false};
    std::atomic<int> frameId_{0};
    std::atomic<int> faceCount_{0};
    std::atomic<int> trackCount_{0};
    std::atomic<bool> bootComplete_{false};
    std::atomic<double> neuralLoad_{0.0};
    std::atomic<int> identityConfidence_{0};
    std::atomic<int> authorizedCount_{0};
    std::atomic<int> lockStrength_{0};
    std::atomic<int> radarBlipCount_{0};

    QString lockPhase_;
    QVariantList threatHistory_;
    QVariantList neuralHistory_;
    QVariantList sectorActivity_;
    QString threatReason_;
    QString cameraStatus_;
    QString faceStatus_;
    QString recognitionEngine_;
    QString targetManifest_;
    QString identityDossier_;
    QString identityStatus_;
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point lastHistoryPush_{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point lastThreatReasonPush_{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point lastTelemetryPush_{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point lastDossierPush_{std::chrono::steady_clock::now()};
    bool pendingTextDirty_{false};
};

}  // namespace csx::ui
