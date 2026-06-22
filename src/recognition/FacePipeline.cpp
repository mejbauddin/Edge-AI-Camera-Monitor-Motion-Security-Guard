#include "FacePipeline.hpp"

#include "ArcFaceEmbedder.hpp"
#include "Database.hpp"
#include "EmbeddingMath.hpp"

#include "CvHelpers.hpp"
#include "Timer.hpp"

#include <cstring>
#include <algorithm>

namespace csx::recognition {

FacePipeline::FacePipeline(std::shared_ptr<csx::database::Database> database,
                           RecognitionSettings settings)
    : database_(std::move(database)),
      settings_(std::move(settings)),
      detector_(settings_),
      embedder_(settings_),
      classifier_(settings_.matchThreshold, settings_.foeThreshold),
      cache_(settings_.cacheCapacity) {
    if (settings_.runtimeDnn) {
        const bool detectorOk = detector_.loadModel(settings_.detectorModel);
        const bool embedderOk = embedder_.loadModel(settings_.embedderModel);
        if (!detectorOk) {
            health_.status = core::EngineStatus::Degraded;
            health_.detail = "YuNet model missing: " + settings_.detectorModel;
        } else if (!embedderOk) {
            health_.status = core::EngineStatus::Degraded;
            health_.detail = "SFace model missing: " + settings_.embedderModel;
        } else {
            health_.status = core::EngineStatus::Online;
            health_.detail = "YuNet + SFace DNN online";
        }
    } else {
        health_.status = core::EngineStatus::Online;
        health_.detail = "Track-guided biometric mode (CPU embeddings)";
    }
    reloadAuthorizedFaces();
}

void FacePipeline::reloadAuthorizedFaces() {
    if (database_ && database_->isOpen()) {
        faceDatabase_.loadAuthorized(database_->faces().listAuthorizedFaces());
    }
}

FaceDatabase& FacePipeline::faceDatabase() noexcept {
    return faceDatabase_;
}

std::string FacePipeline::lockPhase() const {
    if (!hasHeldMatch_) {
        return "SCANNING";
    }
    if (heldFrames_ < 3) {
        return "ACQUIRING";
    }
    if (missedFrames_ > 0) {
        return "HOLDING";
    }
    return "LOCKED";
}

int FacePipeline::lockStrength() const noexcept {
    if (!hasHeldMatch_) {
        return 0;
    }
    return std::min(100, heldFrames_ * 10 + (missedFrames_ == 0 ? 30 : 10) - missedFrames_ * 4);
}

void FacePipeline::updateHeldLock(const std::vector<core::FaceMatch>& rawMatches) {
    if (rawMatches.empty()) {
        tickHeldLock();
        return;
    }

    const auto* best = &rawMatches.front();
    for (const auto& match : rawMatches) {
        if (match.confidence > best->confidence) {
            best = &match;
        }
    }

    heldMatch_ = *best;
    heldMatch_.trackId = 1;
    heldFrames_ = std::min(heldFrames_ + 1, 100);
    missedFrames_ = 0;
    hasHeldMatch_ = true;
}

void FacePipeline::tickHeldLock() {
    if (!hasHeldMatch_) {
        return;
    }
    ++missedFrames_;
    if (missedFrames_ > static_cast<int>(settings_.lockHoldFrames)) {
        hasHeldMatch_ = false;
        heldFrames_ = 0;
        missedFrames_ = 0;
        heldMatch_ = {};
    }
}

std::string FacePipeline::cacheKey(const core::Frame& frame,
                                   const FaceDetection& detection) const {
    const int qx = static_cast<int>(detection.bbox.x / 24);
    const int qy = static_cast<int>(detection.bbox.y / 24);
    return std::to_string(frame.sequence / 3) + ":" + std::to_string(qx) + ":" +
           std::to_string(qy);
}

void FacePipeline::recognize(const core::Frame& frame, const std::vector<core::Track>& tracks,
                             std::vector<core::FaceMatch>& outMatches) {
    ++frameCounter_;

    if (!settings_.enabled || !frame.valid()) {
        outMatches.clear();
        health_.status = core::EngineStatus::Degraded;
        health_.detail = "Recognition disabled or invalid frame";
        return;
    }

    const bool runInference = settings_.inferenceIntervalFrames <= 1 ||
                              frameCounter_ % settings_.inferenceIntervalFrames == 0;

    if (!runInference) {
        tickHeldLock();
        outMatches.clear();
        if (hasHeldMatch_) {
            outMatches.push_back(heldMatch_);
        }
        health_.status = core::EngineStatus::Online;
        health_.detail = "Lock hold — " + lockPhase();
        return;
    }

    csx::utils::Timer timer;

    core::Frame stableFrame = frame;
    if (frame.bgrData && frame.width > 0 && frame.height > 0) {
        const auto byteCount = csx::utils::bgrBufferSize(frame.width, frame.height);
        auto buffer = std::make_shared<std::vector<std::uint8_t>>(byteCount);
        std::memcpy(buffer->data(), frame.bgrData->data(), byteCount);
        stableFrame.bgrData = std::move(buffer);
    }

    const auto detections = detector_.detect(stableFrame, tracks);
    std::vector<core::FaceMatch> rawMatches;
    rawMatches.reserve(detections.size());

    for (const auto& detection : detections) {
        if (detection.bbox.width <= 1.0F || detection.bbox.height <= 1.0F) {
            continue;
        }

        const auto key = cacheKey(stableFrame, detection);
        std::vector<float> embedding;
        if (const auto cached = cache_.get(key)) {
            embedding = *cached;
        } else {
            embedding = embedder_.embed(stableFrame, detection);
            if (!embedding.empty()) {
                cache_.put(key, embedding);
            }
        }

        if (embedding.empty()) {
            continue;
        }

        auto match = classifier_.classify(detection, embedding, faceDatabase_);
        rawMatches.push_back(std::move(match));
    }

    updateHeldLock(rawMatches);
    outMatches.clear();
    if (hasHeldMatch_) {
        outMatches.push_back(heldMatch_);
    }

    ++processedFrames_;
    lastLatencyMs_ = timer.elapsedMs();
    health_.status = core::EngineStatus::Online;
    health_.detail = lockPhase() + " — " + std::to_string(outMatches.size()) + " face(s)";
    health_.confidence = static_cast<float>(lockStrength()) / 100.0F;
    health_.lastLatencyMs = static_cast<float>(lastLatencyMs_);
    health_.processedFrames = processedFrames_;
}

core::EngineHealth FacePipeline::health() const {
    return health_;
}

EnrollmentWizardController::EnrollmentWizardController(
    std::shared_ptr<csx::database::Database> database, std::shared_ptr<FacePipeline> pipeline)
    : database_(std::move(database)), pipeline_(std::move(pipeline)) {}

bool EnrollmentWizardController::beginEnrollment(const std::string& userName, const std::string& role) {
    if (!database_ || !database_->isOpen()) {
        return false;
    }

    const auto userId = database_->users().createUser(userName, role, "", "STANDARD");
    if (!userId.has_value()) {
        return false;
    }

    userId_ = userId;
    userName_ = userName;
    samples_.clear();
    active_ = true;
    return true;
}

bool EnrollmentWizardController::addSample(const core::Frame& frame, const core::Rect2f& faceBox) {
    if (!active_ || !pipeline_) {
        return false;
    }

    ArcFaceEmbedder embedder;
    FaceDetection detection;
    detection.bbox = faceBox;
    detection.score = 1.0F;
    const auto embedding = embedder.embed(frame, detection);
    if (embedding.empty()) {
        return false;
    }

    samples_.push_back(embedding);
    return true;
}

bool EnrollmentWizardController::finalizeEnrollment() {
    if (!active_ || !database_ || !userId_.has_value() || samples_.empty() || !pipeline_) {
        return false;
    }

    const auto embedding = averageEmbeddings(samples_);
    const auto faceId = database_->faces().addAuthorizedFace(*userId_, embedding);
    if (!faceId.has_value()) {
        return false;
    }

    StoredFaceIdentity identity;
    identity.faceId = *faceId;
    identity.userId = *userId_;
    identity.userName = userName_;
    identity.embedding = embedding;
    pipeline_->faceDatabase().addAuthorized(identity);
    pipeline_->reloadAuthorizedFaces();

    samples_.clear();
    userId_.reset();
    userName_.clear();
    active_ = false;
    return true;
}

void EnrollmentWizardController::cancelEnrollment() {
    samples_.clear();
    userId_.reset();
    userName_.clear();
    active_ = false;
}

std::size_t EnrollmentWizardController::sampleCount() const noexcept {
    return samples_.size();
}

bool EnrollmentWizardController::active() const noexcept {
    return active_;
}

std::shared_ptr<FacePipeline> createFacePipeline(std::shared_ptr<csx::database::Database> database,
                                                 RecognitionSettings settings) {
    return std::make_shared<FacePipeline>(std::move(database), std::move(settings));
}

}  // namespace csx::recognition
