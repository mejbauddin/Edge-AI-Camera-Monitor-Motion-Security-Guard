#pragma once

#include "RecognitionSettings.hpp"
#include "types/Frame.hpp"

#include <memory>
#include <string>
#include <vector>

namespace csx::recognition {

class ArcFaceEmbedder {
public:
    explicit ArcFaceEmbedder(RecognitionSettings settings = {});
    ~ArcFaceEmbedder();

    bool loadModel(const std::string& modelPath);
    [[nodiscard]] std::vector<float> embed(const core::Frame& frame,
                                           const FaceDetection& detection) const;

private:
    [[nodiscard]] std::vector<float> embedWithOpenCv(const core::Frame& frame,
                                                     const FaceDetection& detection) const;
    [[nodiscard]] std::vector<float> embedWithCpuFallback(const core::Frame& frame,
                                                          const core::Rect2f& faceBox) const;

    RecognitionSettings settings_;
#if defined(CSX_HAS_OPENCV)
    class RecognizerImpl;
    std::unique_ptr<RecognizerImpl> impl_;
    bool modelLoaded_{false};
#endif
};

}  // namespace csx::recognition
