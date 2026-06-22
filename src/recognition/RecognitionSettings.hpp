#pragma once

#include "types/Frame.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace csx::recognition {

struct RecognitionSettings {
    bool enabled{true};
    bool runtimeDnn{false};
    std::uint32_t inferenceIntervalFrames{12};
    float matchThreshold{0.42F};
    float foeThreshold{0.30F};
    std::uint32_t lockHoldFrames{18};
    float bboxSmoothAlpha{0.38F};
    std::uint32_t classificationVoteWindow{8};
    std::string detectorModel{"assets/models/yunet_2023mar.onnx"};
    std::string embedderModel{"assets/models/sface_2021dec.onnx"};
    std::uint32_t embeddingSize{128};
    std::uint32_t cacheCapacity{256};
};

struct FaceDetection {
    core::Rect2f bbox;
    float score{0.0F};
    std::uint32_t trackId{0};
    std::array<core::Point2f, 5> landmarks{};
    bool hasLandmarks{false};
};

}  // namespace csx::recognition
