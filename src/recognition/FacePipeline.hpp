#pragma once

#include "ArcFaceEmbedder.hpp"
#include "EmbeddingCache.hpp"
#include "FaceDatabase.hpp"
#include "FriendOrFoeClassifier.hpp"
#include "RecognitionSettings.hpp"
#include "YuNetDetector.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>
#include <optional>

namespace csx::database {
class Database;
}

namespace csx::recognition {

class FacePipeline final : public core::IFaceRecognizer {
public:
    FacePipeline(std::shared_ptr<csx::database::Database> database,
                 RecognitionSettings settings = {});

    void recognize(const core::Frame& frame, const std::vector<core::Track>& tracks,
                   std::vector<core::FaceMatch>& outMatches) override;
    [[nodiscard]] core::EngineHealth health() const override;

    void reloadAuthorizedFaces();
    [[nodiscard]] FaceDatabase& faceDatabase() noexcept;
    [[nodiscard]] std::string lockPhase() const;
    [[nodiscard]] int lockStrength() const noexcept;

private:
    [[nodiscard]] std::string cacheKey(const core::Frame& frame, const FaceDetection& detection) const;
    void updateHeldLock(const std::vector<core::FaceMatch>& rawMatches);
    void tickHeldLock();

    std::shared_ptr<csx::database::Database> database_;
    RecognitionSettings settings_;
    FaceDatabase faceDatabase_;
    YuNetDetector detector_;
    ArcFaceEmbedder embedder_;
    FriendOrFoeClassifier classifier_;
    EmbeddingCache cache_;
    core::FaceMatch heldMatch_{};
    int heldFrames_{0};
    int missedFrames_{0};
    bool hasHeldMatch_{false};
    core::EngineHealth health_;
    std::uint64_t frameCounter_{0};
    std::uint64_t processedFrames_{0};
    double lastLatencyMs_{0.0};
};

class EnrollmentWizardController {
public:
    EnrollmentWizardController(std::shared_ptr<csx::database::Database> database,
                               std::shared_ptr<FacePipeline> pipeline);

    bool beginEnrollment(const std::string& userName, const std::string& role = "operator");
    bool addSample(const core::Frame& frame, const core::Rect2f& faceBox);
    bool finalizeEnrollment();
    void cancelEnrollment();

    [[nodiscard]] std::size_t sampleCount() const noexcept;
    [[nodiscard]] bool active() const noexcept;

private:
    std::shared_ptr<csx::database::Database> database_;
    std::shared_ptr<FacePipeline> pipeline_;
    std::optional<std::int64_t> userId_;
    std::string userName_;
    std::vector<std::vector<float>> samples_;
    bool active_{false};
};

std::shared_ptr<FacePipeline> createFacePipeline(std::shared_ptr<csx::database::Database> database,
                                                 RecognitionSettings settings = {});

}  // namespace csx::recognition
