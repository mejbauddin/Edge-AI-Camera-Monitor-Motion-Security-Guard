#pragma once

#include <cstdint>
#include <string>

namespace csx::motion {

struct MotionSettings {
    std::string algorithm{"mog2"};
    double learningRate{0.005};
    int threshold{25};
    int minBlobArea{400};
    bool shadowRemoval{true};
    std::uint32_t processingWidth{640};
    std::uint32_t processingHeight{360};
    int morphKernelSize{3};
    int morphIterations{1};
};

}  // namespace csx::motion
