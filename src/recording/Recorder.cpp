#include "Recorder.hpp"

#include "Database.hpp"
#include "PathResolver.hpp"
#include "Timer.hpp"
#include "types/Enums.hpp"

#include <iomanip>
#include <sstream>

namespace csx::recording {

namespace {

std::string timestampTag() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t raw = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &raw);
#else
    localtime_r(&raw, &localTime);
#endif
    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y%m%d_%H%M%S");
    return stream.str();
}

}  // namespace

Recorder::Recorder(std::shared_ptr<csx::database::Database> database, RecordingSettings settings,
                   csx::utils::PathResolver pathResolver)
    : database_(std::move(database)),
      settings_(std::move(settings)),
      pathResolver_(std::move(pathResolver)),
      rollingBuffer_(settings_),
      storageManager_(settings_) {}

void Recorder::setSettings(const RecordingSettings& settings) {
    settings_ = settings;
    rollingBuffer_.setSettings(settings_);
    storageManager_.setSettings(settings_);
}

const RecordingSettings& Recorder::settings() const noexcept {
    return settings_;
}

RecorderState Recorder::state() const noexcept {
    return state_;
}

std::size_t Recorder::activeClipFrameCount() const noexcept {
    return activeClip_.size();
}

const RollingBuffer& Recorder::rollingBuffer() const noexcept {
    return rollingBuffer_;
}

std::filesystem::path Recorder::outputDirectory() const {
    const auto directory = pathResolver_.resolve(settings_.outputDirectory);
    std::filesystem::create_directories(directory);
    return directory;
}

std::filesystem::path Recorder::buildClipPath(const core::ThreatAssessment& assessment) const {
    const auto directory = outputDirectory();
    const auto fileName = "threat_" + core::toString(assessment.level) + "_" + timestampTag();
    return directory / fileName;
}

BufferedFrame Recorder::toBufferedFrame(const core::Frame& frame) const {
    BufferedFrame buffered;
    buffered.frame = frame;
    buffered.timestamp = frame.captureTime != std::chrono::steady_clock::time_point{}
                           ? frame.captureTime
                           : std::chrono::steady_clock::now();
    if (buffered.frame.bgrData != nullptr) {
        buffered.frame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(*frame.bgrData);
    }
    return buffered;
}

void Recorder::feedFrame(const core::Frame& frame) {
    if (!settings_.enabled || !frame.valid()) {
        health_.status = core::EngineStatus::Degraded;
        health_.detail = "Recording disabled or invalid frame";
        return;
    }

    csx::utils::Timer timer;
    const auto buffered = toBufferedFrame(frame);
    rollingBuffer_.push(buffered);

    if (state_ == RecorderState::Recording) {
        activeClip_.push_back(buffered);

        if (postBufferEnds_ && std::chrono::steady_clock::now() >= *postBufferEnds_) {
            finalizeClip();
        }
    }

    ++fedFrames_;
    health_.status = core::EngineStatus::Online;
    health_.lastLatencyMs = static_cast<float>(timer.elapsedMs());
    health_.processedFrames = fedFrames_;
    health_.detail = state_ == RecorderState::Recording ? "Recording threat clip" : "Rolling buffer active";
}

void Recorder::startRecording() {
    if (state_ == RecorderState::Recording) {
        return;
    }
    
    state_ = RecorderState::Recording;
    recordingStarted_ = std::chrono::steady_clock::now();
    postBufferEnds_.reset();
    
    health_.status = core::EngineStatus::Online;
}

void Recorder::triggerRecording(const core::ThreatAssessment& assessment) {
    if (!settings_.enabled) {
        return;
    }

    if (static_cast<int>(assessment.level) < static_cast<int>(settings_.autoRecordLevel)) {
        return;
    }

    if (state_ == RecorderState::Recording) {
        activeAssessment_ = assessment;
        return;
    }

    state_ = RecorderState::Recording;
    activeAssessment_ = assessment;
    activeClip_ = rollingBuffer_.snapshot();
    if (!activeClip_.empty()) {
        activeCameraId_ = activeClip_.back().frame.cameraId;
    }
    recordingStarted_ = std::chrono::steady_clock::now();
    postBufferEnds_ = *recordingStarted_ + std::chrono::seconds(settings_.postBufferSeconds);

    if (settings_.captureSnapshotOnTrigger && !activeClip_.empty()) {
        const auto snapshotPath = buildClipPath(assessment);
        (void)snapshotCapture_.capturePpm(activeClip_.back().frame, snapshotPath);
    }

    health_.detail = "Threat clip triggered";
}

void Recorder::stopRecording() {
    if (state_ != RecorderState::Recording) {
        return;
    }
    finalizeClip();
}

void Recorder::finalizeClip() {
    if (activeClip_.empty()) {
        state_ = RecorderState::Idle;
        postBufferEnds_.reset();
        recordingStarted_.reset();
        return;
    }

    const auto clipPath = buildClipPath(activeAssessment_);
    const auto result = clipWriter_.writeClip(clipPath, activeClip_, settings_.assumedFps);
    if (result.success) {
        ++savedClips_;
        if (database_ && database_->isOpen()) {
            database_->recordings().insertRecording(result.outputPath.string(), result.durationSeconds,
                                                    core::toString(activeAssessment_.level), activeCameraId_);
            database_->events().insertEvent("recording.started", activeCameraId_,
                                            R"({"format":")" + result.format + R"("})");
        }
        storageManager_.enforceQuota(outputDirectory());
    }

    activeClip_.clear();
    state_ = RecorderState::Idle;
    postBufferEnds_.reset();
    recordingStarted_.reset();
    health_.detail = result.success ? "Clip saved" : "Clip save failed";
}

core::EngineHealth Recorder::health() const {
    return health_;
}

std::shared_ptr<core::IRecorder> createRecorder(std::shared_ptr<csx::database::Database> database,
                                                RecordingSettings settings,
                                                csx::utils::PathResolver pathResolver) {
    return std::make_shared<Recorder>(std::move(database), std::move(settings), std::move(pathResolver));
}

}  // namespace csx::recording
