#include "FriendOrFoeClassifier.hpp"

namespace csx::recognition {

FriendOrFoeClassifier::FriendOrFoeClassifier(const float matchSimilarity, const float foeSimilarity)
    : matchSimilarity_(matchSimilarity), foeSimilarity_(foeSimilarity) {}

core::FaceMatch FriendOrFoeClassifier::classify(const FaceDetection& detection,
                                                const std::vector<float>& embedding,
                                                const FaceDatabase& database) const {
    core::FaceMatch match;
    match.trackId = detection.trackId;
    match.bbox = detection.bbox;
    match.embeddingDistance = 0.0F;
    match.confidence = detection.score;
    match.classification = core::IdentityClassification::Unknown;
    match.identityName = "ANALYZING";

    if (database.authorizedCount() == 0) {
        match.identityName = "UNKNOWN";
        return match;
    }

    const auto analysis = database.analyzeMatch(embedding, matchSimilarity_);
    match.embeddingDistance = 1.0F - analysis.bestSimilarity;

    if (analysis.authorizedMatch.has_value()) {
        const auto& candidate = *analysis.authorizedMatch;
        match.classification = core::IdentityClassification::Authorized;
        match.identityName = candidate.identity.userName;
        match.userId = std::to_string(candidate.identity.userId);
        match.country = candidate.identity.country;
        match.role = candidate.identity.role;
        match.clearance = candidate.identity.clearance;
        match.confidence = std::max(match.confidence, candidate.confidence);
        return match;
    }

    if (analysis.hasClosest && analysis.bestSimilarity <= foeSimilarity_) {
        match.classification = core::IdentityClassification::Foe;
        match.identityName = "UNAUTHORIZED";
        match.confidence = std::max(match.confidence, 1.0F - analysis.bestSimilarity);
        return match;
    }

    match.classification = core::IdentityClassification::Unknown;
    match.identityName = "ANALYZING";
    return match;
}

}  // namespace csx::recognition
