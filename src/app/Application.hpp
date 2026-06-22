#pragma once

#include "recognition/FaceEnrollmentService.hpp"

#include <atomic>
#include <memory>

#ifdef CSX_BUILD_UI
#include <QString>
#include <cstdint>
#include <mutex>
#include <vector>

#include "core/types/Frame.hpp"
#endif

namespace csx::ui {
class DashboardBridge;
class FrameImageProvider;
}  // namespace csx::ui

namespace csx {

#ifdef CSX_BUILD_UI
struct VisionUiSnapshot {
    bool hasFrame{false};
    bool radarOnly{false};
    std::shared_ptr<const std::vector<std::uint8_t>> displayBgr;
    std::uint32_t width{0};
    std::uint32_t height{0};
    core::SystemHealth health{};
    core::ThreatAssessment threat{};
    bool recording{false};
    int faceCount{0};
    int trackCount{0};
    QString cameraStatus;
    QString faceStatus;
    QString recognitionEngine;
    QString targetManifest;
    double neuralLoad{0.0};
    QString identityDossier;
    QString identityStatus;
    int identityConfidence{0};
    int authorizedCount{0};
    QString lockPhase;
    int lockStrength{0};
    int radarBlips{0};
    double threatScore{0.0};
    std::vector<int> sectorActivity;
};
#endif

namespace core {
class IEventBus;
class ILogger;
class ConfigService;
class PipelineOrchestrator;
}  // namespace core

namespace camera {
class CameraEngine;
}

namespace motion {
class MotionEngine;
}

namespace tracking {
class ObjectTracker;
}

namespace recognition {
class FacePipeline;
}

namespace behavior {
class BehaviorEngine;
}

namespace threat {
class ThreatEngine;
}

namespace recording {
class Recorder;
}

namespace database {
class Database;
}

namespace hud {
class HudRenderer;
}

namespace radar {
class RadarModel;
}

namespace voice {
class VoiceService;
}

namespace alerts {
class AlertManager;
class NotificationCenter;
}

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void shutdown();
    void run();

#ifdef CSX_BUILD_UI
    int runWithUi(int argc, char* argv[]);
    bool captureVisionSnapshot(VisionUiSnapshot& out);
    void applyVisionSnapshot(ui::DashboardBridge* bridge, ui::FrameImageProvider* frames,
                             const VisionUiSnapshot& snapshot);
    radar::RadarModel* radarModel() { return radarModel_.get(); }
#endif

    core::IEventBus* eventBus() const { return eventBus_.get(); }

    recognition::EnrollmentResult enrollFromImage(const std::string& path, const std::string& name,
                                                  const std::string& country,
                                                  const std::string& role = "operator");
    recognition::EnrollmentResult enrollFromLiveFrame(const std::string& name,
                                                      const std::string& country,
                                                      const std::string& role = "operator");
    std::size_t importAuthorizedFaces(const std::string& folderPath);
    [[nodiscard]] std::size_t authorizedFaceCount() const;
    [[nodiscard]] std::string authorizedDatabaseSummary() const;

private:
    void initializeLogging();
    void initializeConfig();
    void initializeDatabase();
    void initializeRecording();
    void initializeCamera();
    void initializeVisionPipeline();
    void initializeAI();
    void initializeUI();
    void wireDependencies();

    std::shared_ptr<core::IEventBus> eventBus_;
    std::shared_ptr<core::ILogger> logger_;
    std::unique_ptr<core::ConfigService> configService_;
    std::unique_ptr<core::PipelineOrchestrator> orchestrator_;

    std::unique_ptr<camera::CameraEngine> cameraEngine_;
    std::unique_ptr<motion::MotionEngine> motionEngine_;

    std::unique_ptr<tracking::ObjectTracker> objectTracker_;
    std::unique_ptr<recognition::FacePipeline> facePipeline_;
    std::unique_ptr<recognition::FaceEnrollmentService> enrollmentService_;
    core::Frame lastFrame_;
#ifdef CSX_BUILD_UI
    mutable std::mutex lastFrameMutex_;
#endif
    std::unique_ptr<behavior::BehaviorEngine> behaviorEngine_;
    std::unique_ptr<threat::ThreatEngine> threatEngine_;

    std::unique_ptr<recording::Recorder> recorder_;
    std::shared_ptr<database::Database> database_;

    std::unique_ptr<hud::HudRenderer> hudRenderer_;

    std::unique_ptr<radar::RadarModel> radarModel_;
    std::unique_ptr<voice::VoiceService> voiceService_;
    std::unique_ptr<alerts::AlertManager> alertManager_;
    std::unique_ptr<alerts::NotificationCenter> notificationCenter_;

    std::atomic<bool> running_{false};
    std::atomic<bool> shutdown_{false};
};

}  // namespace csx
