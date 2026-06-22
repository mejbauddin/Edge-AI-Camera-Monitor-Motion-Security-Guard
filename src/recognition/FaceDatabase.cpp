#include "FaceDatabase.hpp"

#include <algorithm>

namespace csx::recognition {

void FaceDatabase::loadAuthorized(const std::vector<csx::database::AuthorizedFaceRecord>& records) {
    authorized_.clear();
    authorized_.reserve(records.size());
    for (const auto& record : records) {
        StoredFaceIdentity identity;
        identity.faceId = record.id;
        identity.userId = record.userId;
        identity.userName = record.userName;
        identity.role = record.role;
        identity.country = record.country;
        identity.clearance = record.clearance;
        identity.embedding = record.embedding;
        normalizeEmbedding(identity.embedding);
        authorized_.push_back(std::move(identity));
    }
}

void FaceDatabase::addAuthorized(const StoredFaceIdentity& identity) {
    authorized_.push_back(identity);
    normalizeEmbedding(authorized_.back().embedding);
}

void FaceDatabase::clear() {
    authorized_.clear();
}

FaceMatchAnalysis FaceDatabase::analyzeMatch(const std::vector<float>& embedding,
                                             const float authorizeSimilarity) const {
    FaceMatchAnalysis analysis;
    if (authorized_.empty()) {
        return analysis;
    }

    for (const auto& identity : authorized_) {
        const float similarity = cosineSimilarity(embedding, identity.embedding);
        if (!analysis.hasClosest || similarity > analysis.bestSimilarity) {
            analysis.hasClosest = true;
            analysis.bestSimilarity = similarity;
            analysis.closestMatch.identity = identity;
            analysis.closestMatch.similarity = similarity;
            analysis.closestMatch.confidence = std::max(0.0F, similarity);
        }
    }

    if (analysis.hasClosest && analysis.bestSimilarity >= authorizeSimilarity) {
        analysis.authorizedMatch = analysis.closestMatch;
    }

    return analysis;
}

std::optional<FaceMatchCandidate> FaceDatabase::findBestMatch(const std::vector<float>& embedding,
                                                              const float minSimilarity) const {
    const auto analysis = analyzeMatch(embedding, minSimilarity);
    return analysis.authorizedMatch;
}

std::size_t FaceDatabase::authorizedCount() const noexcept {
    return authorized_.size();
}

}  // namespace csx::recognition
