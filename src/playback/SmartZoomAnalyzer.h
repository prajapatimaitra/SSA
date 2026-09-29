#pragma once

#include <vector>
#include "project/ProjectMetadata.h"
#include "project/CameraKeyframe.h"

namespace ssa::playback {

class SmartZoomAnalyzer {
public:
    static std::vector<project::CameraKeyframe> generateKeyframes(
        const project::ProjectMetadata& metadata, 
        int targetWidth, 
        int targetHeight);
};

} // namespace ssa::playback
