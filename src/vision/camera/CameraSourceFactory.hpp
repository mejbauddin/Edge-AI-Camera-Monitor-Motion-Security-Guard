#pragma once

#include "CameraTypes.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>
#include <string>

namespace csx::camera {

std::unique_ptr<core::ICameraSource> createCameraSource(const std::string& sourceUri,
                                                        CameraSettings settings = {});

}  // namespace csx::camera
