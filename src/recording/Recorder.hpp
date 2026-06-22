#pragma once

#include "Mp4Writer.hpp"
#include "PathResolver.hpp"
#include "RecordingSettings.hpp"
#include "RollingBuffer.hpp"
#include "SnapshotCapture.hpp"
#include "StorageManager.hpp"
#include "interfaces/Interfaces.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace csx::database {
class Database;
}

namespace csx::recording {

enum class RecorderState : std::uint8_t { Idle = 0, Recording = 1 };

class Recorder final : public core::IRecorder {
public:
    Recorder(std::shared_ptr<csx::database::Database> database, RecordingSettings settings,
             csx::utils::PathResolver pathResolver = csx::utils::PathResolver{});

    void feedFrame(const core::Frame& frame) override;
    void triggerRecording(const core::ThreatAssessment& assessment) override;
    void startRecording() override;
    void stopRecording() override;
    [[nodiscard]] core::EngineHealth health() const override;

    void setSettings(const RecordingSettings& settings);
    [[nodiscard]] const RecordingSettings& settings() const noexcept;
    [[nodiscard]] RecorderState state() const noexcept;
    [[nodiscard]] std::size_t activeClipFrameCount() const noexcept;
    [[nodiscard]] const RollingBuffer& rollingBuffer() const noexcept;

private:
    [[nodiscard]] std::filesystem::path outputDirectory() const;
    [[nodiscard]] std::filesystem::path buildClipPath(const core::ThreatAssessment& assessment) const;
    void finalizeClip();
    BufferedFrame toBufferedFrame(const core::Frame& frame) const;

    std::shared_ptr<csx::database::Database> database_;
    RecordingSettings settings_;
    csx::utils::PathResolver pathResolver_;
    RollingBuffer rollingBuffer_;
    StorageManager storageManager_;
    SnapshotCapture snapshotCapture_;
    Mp4Writer clipWriter_;
    core::EngineHealth health_;

    RecorderState state_{RecorderState::Idle};
    std::vector<BufferedFrame> activeClip_;
    std::optional<std::chrono::steady_clock::time_point> recordingStarted_;
    std::optional<std::chrono::steady_clock::time_point> postBufferEnds_;
    core::ThreatAssessment activeAssessment_;
    std::string activeCameraId_;
    std::uint64_t fedFrames_{0};
    std::uint64_t savedClips_{0};
};

std::shared_ptr<core::IRecorder> createRecorder(std::shared_ptr<csx::database::Database> database,
                                                RecordingSettings settings = {},
                                                csx::utils::PathResolver pathResolver = csx::utils::PathResolver{});

}  // namespace csx::recording
