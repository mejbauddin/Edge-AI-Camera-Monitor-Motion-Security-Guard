#include "Application.hpp"

#include "core/config/ConfigService.hpp"
#include "core/events/EventBusFactory.hpp"
#include "core/logging/Logger.hpp"
#include "core/pipeline/PipelineOrchestrator.hpp"
#include "vision/camera/CameraEngine.hpp"
#include "vision/camera/CameraTypes.hpp"
#include "vision/motion/MotionEngine.hpp"
#include "vision/motion/MotionSettings.hpp"
#include "tracking/ObjectTracker.hpp"
#include "recognition/FacePipeline.hpp"
#include "recognition/FaceEnrollmentService.hpp"
#include "recognition/RecognitionSettings.hpp"
#include "behavior/BehaviorEngine.hpp"
#include "threat/ThreatEngine.hpp"
#include "recording/Recorder.hpp"
#include "recording/RecordingSettings.hpp"
#include "database/Database.hpp"
#include "ui/hud/HudRenderer.hpp"
#include "radar/RadarModel.hpp"
#include "voice/VoiceService.hpp"
#include "alerts/AlertManager.hpp"
#include "alerts/NotificationCenter.hpp"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

#ifdef CSX_BUILD_UI
#include "UiBootstrap.hpp"
#include "ui/bridge/DashboardBridge.hpp"
#include "ui/providers/FrameImageProvider.hpp"
#include "recording/Recorder.hpp"
#include <QStringList>
#endif

namespace csx {

Application::Application() = default;

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    try {
        initializeLogging();
        initializeConfig();
        initializeDatabase();
        initializeRecording();
        initializeCamera();
        initializeVisionPipeline();
        initializeAI();
        initializeUI();
        wireDependencies();

        logger_->info("Application", "Cyber Sentinel X initialized successfully");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void Application::shutdown() {
    if (shutdown_.exchange(true)) {
        return;
    }

    running_.store(false, std::memory_order_release);

    if (cameraEngine_) {
        cameraEngine_->stop();
    }
    if (orchestrator_) {
        orchestrator_->stop();
    }

    notificationCenter_.reset();
    alertManager_.reset();
    voiceService_.reset();
    radarModel_.reset();
    hudRenderer_.reset();
    recorder_.reset();
    threatEngine_.reset();
    behaviorEngine_.reset();
    facePipeline_.reset();
    enrollmentService_.reset();
    objectTracker_.reset();
        motionEngine_.reset();
        cameraEngine_.reset();
    orchestrator_.reset();
    configService_.reset();
    database_.reset();
    logger_.reset();
    eventBus_.reset();
}

void Application::run() {
    running_.store(true, std::memory_order_release);

    while (running_.load(std::memory_order_acquire)) {
        if (cameraEngine_) {
            core::Frame frame;
            if (cameraEngine_->popFrame(frame, std::chrono::milliseconds{16})) {
                if (recorder_) {
                    recorder_->feedFrame(frame);
                }
            }
        }

        if (radarModel_) {
            radarModel_->update();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Application::initializeLogging() {
    eventBus_ = core::createEventBus();
    logger_ = core::createLogger("logs");
    logger_->info("Application", "Initializing Cyber Sentinel X...");
}

void Application::initializeConfig() {
    configService_ = std::make_unique<core::ConfigService>(eventBus_);
    if (!configService_->load("config/default.json")) {
        throw std::runtime_error("Failed to load config/default.json");
    }

    const auto logLevel = configService_->getString("logging.level", "info");
    if (auto* logger = dynamic_cast<core::Logger*>(logger_.get())) {
        logger->setLevel(logLevel);
    }
}

void Application::initializeDatabase() {
    std::filesystem::create_directories("data");

    database_ = std::make_shared<database::Database>();
    const auto dbPath = configService_->getString("database.path", "data/cyber_sentinel.db");
    if (!database_->open(dbPath)) {
        throw std::runtime_error("Failed to open database at " + dbPath);
    }
}

void Application::initializeRecording() {
    recording::RecordingSettings settings;
    settings.enabled = configService_->getBool("recording.enabled", true);
    settings.preBufferSeconds =
        static_cast<std::uint32_t>(configService_->getInt("recording.pre_buffer_seconds", 5));
    settings.postBufferSeconds =
        static_cast<std::uint32_t>(configService_->getInt("recording.post_buffer_seconds", 10));
    settings.codec = configService_->getString("recording.codec", "h264");
    settings.outputDirectory = configService_->getString("recording.output_dir", "recordings");
    settings.maxStorageGb = static_cast<double>(configService_->getFloat("recording.max_storage_gb", 50.0F));

    std::filesystem::create_directories(settings.outputDirectory);
    recorder_ = std::make_unique<recording::Recorder>(database_, settings);
}

void Application::initializeCamera() {
    cameraEngine_ = std::make_unique<camera::CameraEngine>(logger_, eventBus_);

    camera::CameraSettings settings;
    settings.width = static_cast<std::uint32_t>(configService_->getInt("camera.width", 1280));
    settings.height = static_cast<std::uint32_t>(configService_->getInt("camera.height", 720));
    settings.targetFps = static_cast<std::uint32_t>(configService_->getInt("camera.fps", 60));
    settings.bufferDepth = static_cast<std::uint32_t>(configService_->getInt("camera.buffer_depth", 3));
    settings.reconnectIntervalMs =
        static_cast<std::uint32_t>(configService_->getInt("camera.reconnect_interval_ms", 2000));
    settings.maxReconnectAttempts =
        static_cast<std::uint32_t>(configService_->getInt("camera.max_reconnect_attempts", 0));
    cameraEngine_->configure(settings);

    const auto source = configService_->getString("camera.default_source", "synthetic:0");
    if (!cameraEngine_->start(source)) {
        logger_->warn("Application", "Camera start failed for '" + source + "', using synthetic:0");
        if (!cameraEngine_->start("synthetic:0")) {
            throw std::runtime_error("Failed to start camera");
        }
    }
}

void Application::initializeVisionPipeline() {
    orchestrator_ = std::make_unique<core::PipelineOrchestrator>(logger_, eventBus_);
    orchestrator_->start();

    motion::MotionSettings motionSettings;
    motionSettings.algorithm = configService_->getString("motion.algorithm", "mog2");
    motionSettings.learningRate = configService_->getFloat("motion.learning_rate", 0.005F);
    motionSettings.threshold = configService_->getInt("motion.threshold", 25);
    motionSettings.minBlobArea = configService_->getInt("motion.min_blob_area", 400);
    motionSettings.shadowRemoval = configService_->getBool("motion.shadow_removal", true);
    motionEngine_ = std::make_unique<motion::MotionEngine>(motionSettings);
}

void Application::initializeAI() {
    objectTracker_ = std::make_unique<tracking::ObjectTracker>();
    recognition::RecognitionSettings recognitionSettings;
    recognitionSettings.enabled = configService_->getBool("recognition.enabled", true);
    recognitionSettings.runtimeDnn = configService_->getBool("recognition.runtime_dnn", false);
    recognitionSettings.inferenceIntervalFrames =
        static_cast<std::uint32_t>(configService_->getInt("recognition.inference_interval_frames", 12));
    recognitionSettings.matchThreshold =
        configService_->getFloat("recognition.match_threshold", 0.42F);
    recognitionSettings.foeThreshold =
        configService_->getFloat("recognition.foe_threshold", 0.30F);
    recognitionSettings.lockHoldFrames =
        static_cast<std::uint32_t>(configService_->getInt("recognition.lock_hold_frames", 18));
    recognitionSettings.detectorModel =
        configService_->getString("recognition.models.face_detector", "assets/models/yunet_2023mar.onnx");
    recognitionSettings.embedderModel =
        configService_->getString("recognition.models.face_embedder", "assets/models/sface_2021dec.onnx");
    facePipeline_ = std::make_unique<recognition::FacePipeline>(database_, recognitionSettings);
    behaviorEngine_ = std::make_unique<behavior::BehaviorEngine>();
    threatEngine_ = std::make_unique<threat::ThreatEngine>();

    enrollmentService_ =
        std::make_unique<recognition::FaceEnrollmentService>(database_, facePipeline_.get(),
                                                             recognitionSettings);

    const auto importDir = configService_->getString("recognition.import_dir", "assets/faces/authorized");
    if (enrollmentService_->authorizedCount() == 0) {
        const auto imported = enrollmentService_->importAuthorizedFolder(importDir);
        if (imported > 0) {
            logger_->info("Application", "Imported " + std::to_string(imported) + " authorized face(s)");
        }
    }
    facePipeline_->reloadAuthorizedFaces();
}

void Application::initializeUI() {
    hud::HudSettings hudSettings;
    hudSettings.showGrid = configService_->getBool("hud.enabled", true);
    hudSettings.showReticles = true;
    hudSettings.showVelocityVectors = configService_->getBool("hud.show_velocity_vectors", true);
    hudSettings.showPredictionVectors = configService_->getBool("hud.show_prediction_trails", true);
    hudSettings.showScanline = true;
    hudSettings.showThreatMeter = true;
    hudSettings.showDefcon = true;
    hudSettings.showTelemetry = true;
    hudSettings.enableGlitchOnThreat = configService_->getBool("ui.glitch_on_threat", true);
    hudSettings.scanlineSpeed = configService_->getFloat("hud.scanline_speed", 2.4F);
    hudRenderer_ = std::make_unique<hud::HudRenderer>(hudSettings);

    radar::RadarSettings radarSettings;
    radarSettings.sweepRpm = configService_->getFloat("ui.radar_sweep_rpm", 4.0F);
    radarModel_ = std::make_unique<radar::RadarModel>(radarSettings);
    voiceService_ = std::make_unique<voice::VoiceService>();
    alertManager_ = std::make_unique<alerts::AlertManager>();
    notificationCenter_ = std::make_unique<alerts::NotificationCenter>();
}

void Application::wireDependencies() {
    alertManager_->setVoiceService(voiceService_.get());
    alertManager_->setRecorder(recorder_.get());

    notificationCenter_->subscribe([](const alerts::Alert& alert) {
        std::cout << "[ALERT] " << alert.message << std::endl;
    });
}

recognition::EnrollmentResult Application::enrollFromImage(const std::string& path,
                                                           const std::string& name,
                                                           const std::string& country,
                                                           const std::string& role) {
    if (!enrollmentService_) {
        return {false, "Enrollment service unavailable"};
    }
    return enrollmentService_->enrollFromImageFile(path, name, country, role);
}

recognition::EnrollmentResult Application::enrollFromLiveFrame(const std::string& name,
                                                               const std::string& country,
                                                               const std::string& role) {
    if (!enrollmentService_) {
        return {false, "Enrollment service unavailable"};
    }
    if (!lastFrame_.valid()) {
        return {false, "No camera frame available — wait for live feed"};
    }
    core::Frame frameCopy;
    {
#ifdef CSX_BUILD_UI
        std::lock_guard<std::mutex> lock(lastFrameMutex_);
#endif
        frameCopy = lastFrame_;
    }
    return enrollmentService_->enrollFromFrame(frameCopy, name, country, role);
}

std::size_t Application::importAuthorizedFaces(const std::string& folderPath) {
    if (!enrollmentService_) {
        return 0;
    }
    const auto count = enrollmentService_->importAuthorizedFolder(folderPath);
    if (facePipeline_) {
        facePipeline_->reloadAuthorizedFaces();
    }
    return count;
}

std::size_t Application::authorizedFaceCount() const {
    return enrollmentService_ ? enrollmentService_->authorizedCount() : 0;
}

std::string Application::authorizedDatabaseSummary() const {
    if (!database_ || !database_->isOpen()) {
        return "Database offline";
    }
    const auto faces = database_->faces().listAuthorizedFaces();
    if (faces.empty()) {
        return "No authorized profiles — enroll via BIOMETRIC VAULT panel";
    }
    std::ostringstream oss;
    for (const auto& face : faces) {
        oss << face.userName;
        if (!face.country.empty()) {
            oss << " [" << face.country << "]";
        }
        if (!face.role.empty()) {
            oss << " (" << face.role << ")";
        }
        oss << "\n";
    }
    return oss.str();
}

#ifdef CSX_BUILD_UI
namespace {

std::vector<core::Track> buildTracksFromFaces(std::vector<core::FaceMatch>& faces) {
    std::vector<core::Track> tracks;
    tracks.reserve(faces.size());
    std::uint32_t nextId = 1;
    for (auto& face : faces) {
        core::Track track;
        track.id = nextId;
        face.trackId = nextId++;
        track.bbox = face.bbox;
        track.center = face.bbox.center();
        track.confidence = face.confidence;
        track.active = true;
        track.ageFrames = 1;
        tracks.push_back(track);
    }
    return tracks;
}

QString buildTargetManifest(const std::vector<core::FaceMatch>& faces) {
    if (faces.empty()) {
        return QStringLiteral("SCANNING SECTOR...");
    }
    QStringList lines;
    for (const auto& face : faces) {
        QString label;
        switch (face.classification) {
            case core::IdentityClassification::Authorized:
                label = QStringLiteral("◉ AUTHORIZED: %1").arg(QString::fromStdString(face.identityName));
                if (!face.country.empty()) {
                    label += QStringLiteral("  ORIGIN: %1").arg(QString::fromStdString(face.country));
                }
                break;
            case core::IdentityClassification::Foe:
                label = QStringLiteral("✖ FOE — UNAUTHORIZED INTRUDER");
                break;
            default:
                label = QStringLiteral("◌ ANALYZING BIO-SIGNATURE...");
                break;
        }
        lines << QStringLiteral("TGT-%1  %2  CONF %3%")
                     .arg(face.trackId)
                     .arg(label)
                     .arg(static_cast<int>(face.confidence * 100.0F));
    }
    return lines.join(QStringLiteral("\n"));
}

QString buildIdentityDossier(const std::vector<core::FaceMatch>& faces) {
    if (faces.empty()) {
        return QStringLiteral("AWAITING TARGET ACQUISITION");
    }
    const auto& face = faces.front();
    QStringList lines;
    switch (face.classification) {
        case core::IdentityClassification::Authorized:
            lines << QStringLiteral("DESIGNATION: %1").arg(QString::fromStdString(face.identityName));
            if (!face.country.empty()) {
                lines << QStringLiteral("ORIGIN: %1").arg(QString::fromStdString(face.country));
            }
            if (!face.role.empty()) {
                lines << QStringLiteral("ROLE: %1").arg(QString::fromStdString(face.role));
            }
            if (!face.clearance.empty()) {
                lines << QStringLiteral("CLEARANCE: %1").arg(QString::fromStdString(face.clearance));
            }
            lines << QStringLiteral("STATUS: AUTHORIZED — IDENTITY LOCKED");
            break;
        case core::IdentityClassification::Foe:
            lines << QStringLiteral("DESIGNATION: UNAUTHORIZED PERSON");
            lines << QStringLiteral("ORIGIN: UNKNOWN / NOT IN VAULT");
            lines << QStringLiteral("STATUS: ACCESS DENIED — NO BIOMETRIC MATCH");
            break;
        default:
            lines << QStringLiteral("DESIGNATION: %1").arg(QString::fromStdString(face.identityName));
            lines << QStringLiteral("STATUS: ANALYZING — AWAITING CONFIRMATION");
            break;
    }
    lines << QStringLiteral("CONFIDENCE: %1%").arg(static_cast<int>(face.confidence * 100.0F));
    return lines.join(QStringLiteral("\n"));
}

QString cameraStatusLabel(const core::Frame& frame) {
    if (frame.cameraId.rfind("synthetic", 0) == 0) {
        return QStringLiteral("SIMULATED FEED");
    }
    return QStringLiteral("OPTICAL SENSOR ONLINE — %1")
        .arg(QString::fromStdString(frame.cameraId));
}

}  // namespace

int Application::runWithUi(int argc, char* argv[]) {
    return ui::runUi(*this, argc, argv);
}

bool Application::captureVisionSnapshot(VisionUiSnapshot& out) {
    core::Frame frame;
    if (!cameraEngine_ || !cameraEngine_->popFrame(frame, std::chrono::milliseconds{0})) {
        if (radarModel_) {
            radarModel_->update();
            out.radarOnly = true;
            out.radarBlips = static_cast<int>(radarModel_->getBlips().size());
        }
        return out.radarOnly;
    }
    if (!frame.valid()) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(lastFrameMutex_);
        lastFrame_ = frame;
    }

    std::vector<core::Rect2f> blobs;
    if (motionEngine_) {
        motionEngine_->process(frame, blobs);
    }

    std::vector<core::Track> motionTracks;
    if (objectTracker_) {
        objectTracker_->update(frame, blobs, motionTracks);
    }

    std::vector<core::FaceMatch> faces;
    if (facePipeline_ && configService_->getBool("recognition.enabled", true)) {
        facePipeline_->recognize(frame, motionTracks, faces);
    }

    auto tracks = buildTracksFromFaces(faces);
    for (auto& motionTrack : motionTracks) {
        if (!motionTrack.active) {
            continue;
        }
        bool overlapsFace = false;
        for (const auto& faceTrack : tracks) {
            if (faceTrack.id == motionTrack.id) {
                overlapsFace = true;
                break;
            }
        }
        if (!overlapsFace) {
            tracks.push_back(motionTrack);
        }
    }

    std::vector<core::BehaviorAnomaly> anomalies;
    if (behaviorEngine_) {
        behaviorEngine_->analyze(frame, tracks, anomalies);
    }

    core::ThreatAssessment assessment;
    if (threatEngine_) {
        assessment = threatEngine_->assess(tracks, faces, anomalies);
    }

    core::SystemHealth health = orchestrator_ ? orchestrator_->systemHealth() : core::SystemHealth{};
    health.fps = frame.diagnostics.fps;
    health.frameTimeMs = frame.diagnostics.latencyMs;
    if (facePipeline_) {
        health.recognition = facePipeline_->health();
    }
    if (orchestrator_) {
        orchestrator_->setSystemHealth(health);
    }

    core::Frame displayFrame;
    if (hudRenderer_) {
        if (recorder_) {
            hudRenderer_->setRecording(recorder_->state() == recording::RecorderState::Recording);
        }
        hudRenderer_->render(frame, tracks, faces, anomalies, assessment, health, displayFrame);
        out.displayBgr = displayFrame.bgrData;
        out.width = displayFrame.width;
        out.height = displayFrame.height;
    } else {
        out.displayBgr = frame.bgrData;
        out.width = frame.width;
        out.height = frame.height;
    }

    out.hasFrame = true;
    out.health = health;
    out.threat = assessment;
    out.recording = recorder_ && recorder_->state() == recording::RecorderState::Recording;

    const QString lockPhase =
        facePipeline_ ? QString::fromStdString(facePipeline_->lockPhase()) : QStringLiteral("SCANNING");
    const int lockStrength = facePipeline_ ? facePipeline_->lockStrength() : 0;

    QString faceStatus;
    if (faces.empty()) {
        faceStatus = lockPhase == QStringLiteral("HOLDING")
                         ? QStringLiteral("BIOMETRIC HOLD — REACQUIRING")
                         : QStringLiteral("SCANNING FOR BIO-SIGNATURES");
    } else {
        faceStatus = QStringLiteral("BIOMETRIC %1 — %2 TARGET(S)")
                         .arg(lockPhase)
                         .arg(faces.size());
    }

    out.faceCount = static_cast<int>(faces.size());
    out.trackCount = static_cast<int>(tracks.size());
    out.cameraStatus = cameraStatusLabel(frame);
    out.faceStatus = faceStatus;
    out.recognitionEngine =
        facePipeline_ ? QString::fromStdString(facePipeline_->health().detail)
                      : QStringLiteral("OFFLINE");
    out.targetManifest = buildTargetManifest(faces);
    out.neuralLoad = facePipeline_ ? static_cast<double>(facePipeline_->health().lastLatencyMs) : 0.0;
    out.identityDossier = buildIdentityDossier(faces);
    out.identityStatus = faces.empty()
                             ? QStringLiteral("UNKNOWN")
                             : faces.front().classification == core::IdentityClassification::Foe
                                   ? QStringLiteral("UNAUTHORIZED")
                                   : faces.front().classification ==
                                             core::IdentityClassification::Authorized
                                         ? QStringLiteral("AUTHORIZED")
                                         : QStringLiteral("ANALYZING");
    out.identityConfidence =
        faces.empty() ? 0 : static_cast<int>(faces.front().confidence * 100.0F);
    out.authorizedCount = static_cast<int>(authorizedFaceCount());
    out.lockPhase = lockPhase;
    out.lockStrength = lockStrength;
    out.threatScore = assessment.threatScore;

    if (radarModel_) {
        radarModel_->updateTracks(tracks, faces, assessment);
        radarModel_->update();
        out.radarBlips = static_cast<int>(radarModel_->getBlips().size());

        out.sectorActivity.assign(8, 0);
        for (const auto& blip : radarModel_->getBlips()) {
            if (blip.isPrediction) {
                continue;
            }
            const float normalized =
                (blip.coord.angle + static_cast<float>(M_PI)) / (2.f * static_cast<float>(M_PI));
            int sector = static_cast<int>(normalized * 8.f);
            if (sector >= 8) {
                sector = 7;
            }
            if (sector < 0) {
                sector = 0;
            }
            ++out.sectorActivity[static_cast<std::size_t>(sector)];
        }
    }

    if (recorder_) {
        recorder_->feedFrame(frame);
    }

    return true;
}

void Application::applyVisionSnapshot(ui::DashboardBridge* bridge, ui::FrameImageProvider* frames,
                                      const VisionUiSnapshot& snapshot) {
    if (!bridge || !frames) {
        return;
    }

    if (snapshot.hasFrame) {
        frames->updateFrame(snapshot.displayBgr, snapshot.width, snapshot.height);
        bridge->notifyFrameUpdated();
        bridge->updateHealth(snapshot.health);
        bridge->updateThreat(snapshot.threat);
        bridge->setRecording(snapshot.recording);
        bridge->updateVisionTelemetry(snapshot.faceCount, snapshot.trackCount, snapshot.cameraStatus,
                                      snapshot.faceStatus, snapshot.recognitionEngine,
                                      snapshot.targetManifest, snapshot.neuralLoad);
        bridge->updateIdentityDossier(snapshot.identityDossier, snapshot.identityStatus,
                                      snapshot.identityConfidence, snapshot.authorizedCount);
        bridge->updateAnalytics(snapshot.lockPhase, snapshot.lockStrength, snapshot.radarBlips,
                                snapshot.threatScore, snapshot.neuralLoad, snapshot.sectorActivity);
        return;
    }

    if (snapshot.radarOnly) {
        bridge->updateAnalytics(QStringLiteral("SCANNING"), 0, snapshot.radarBlips, 0.0, 0.0,
                                snapshot.sectorActivity);
        bridge->notifyFrameUpdated();
    }
}
#endif

}  // namespace csx
