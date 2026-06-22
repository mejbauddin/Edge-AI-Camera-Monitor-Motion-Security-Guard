#pragma once

#include "FaceDatabase.hpp"
#include "RecognitionSettings.hpp"
#include "types/Frame.hpp"

namespace csx::recognition {

class FriendOrFoeClassifier {
public:
    explicit FriendOrFoeClassifier(float matchSimilarity = 0.42F, float foeSimilarity = 0.30F);

    core::FaceMatch classify(const FaceDetection& detection, const std::vector<float>& embedding,
                             const FaceDatabase& database) const;

private:
    float matchSimilarity_;
    float foeSimilarity_;
};

}  // namespace csx::recognition
