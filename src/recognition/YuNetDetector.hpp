#pragma once

#include "RecognitionSettings.hpp"
#include "types/Frame.hpp"

#include <memory>
#include <string>
#include <vector>

namespace csx::recognition {

class YuNetDetector {
public:
    explicit YuNetDetector(RecognitionSettings settings = {});
    ~YuNetDetector();

    bool loadModel(const std::string& modelPath);
    std::vector<FaceDetection> detect(const core::Frame& frame,
                                      const std::vector<core::Track>& tracks) const;

private:
    std::vector<FaceDetection> detectWithFaceDetectorYn(const core::Frame& frame) const;
    std::vector<FaceDetection> detectFromTracks(const std::vector<core::Track>& tracks) const;

    RecognitionSettings settings_;
#if defined(CSX_HAS_OPENCV)
    class FaceDetectorImpl;
    std::unique_ptr<FaceDetectorImpl> impl_;
    bool modelLoaded_{false};
    mutable std::uint32_t lastWidth_{0};
    mutable std::uint32_t lastHeight_{0};
#endif
};

}  // namespace csx::recognition
