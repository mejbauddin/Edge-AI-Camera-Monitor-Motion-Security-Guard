#pragma once

#include "EmbeddingMath.hpp"
#include "schema/Schema.hpp"
#include "types/Frame.hpp"

#include <optional>
#include <string>
#include <vector>

namespace csx::recognition {

struct StoredFaceIdentity {
    std::int64_t faceId{0};
    std::int64_t userId{0};
    std::string userName;
    std::string role;
    std::string country;
    std::string clearance;
    std::vector<float> embedding;
};

struct FaceMatchCandidate {
    StoredFaceIdentity identity;
    float similarity{0.0F};
    float confidence{0.0F};
};

struct FaceMatchAnalysis {
    std::optional<FaceMatchCandidate> authorizedMatch;
    FaceMatchCandidate closestMatch;
    bool hasClosest{false};
    float bestSimilarity{0.0F};
};

class FaceDatabase {
public:
    void loadAuthorized(const std::vector<csx::database::AuthorizedFaceRecord>& records);
    void addAuthorized(const StoredFaceIdentity& identity);
    void clear();

    [[nodiscard]] std::optional<FaceMatchCandidate> findBestMatch(const std::vector<float>& embedding,
                                                                  float minSimilarity) const;
    [[nodiscard]] FaceMatchAnalysis analyzeMatch(const std::vector<float>& embedding,
                                                 float authorizeSimilarity) const;
    [[nodiscard]] std::size_t authorizedCount() const noexcept;

private:
    std::vector<StoredFaceIdentity> authorized_;
};

}  // namespace csx::recognition
