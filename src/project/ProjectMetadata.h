#pragma once

#include <string>
#include <vector>
#include "input/MouseEvent.h"

namespace ssa::project {

struct ProjectMetadata {
    std::string uuid;
    uint64_t createdAt; // timestamp in microseconds
    uint64_t recordingStartTimestamp; // Mach absolute time of the first video frame
    int videoWidth;
    int videoHeight;
    std::string cursorColor = "#ff3333";
    double cursorScale = 1.0;
    std::string backgroundColor = "#000000";
    uint64_t trimStartTimeMs = 0;
    uint64_t trimEndTimeMs = 0;
    std::vector<input::MouseEvent> mouseEvents;

    struct Marker {
        uint64_t timestampMs;
        std::string color;
        std::string note;
    };
    std::vector<Marker> markers;
};

} // namespace ssa::project
