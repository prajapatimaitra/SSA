#pragma once

#include <string>

namespace ssa::ai {

class IAIModel {
public:
    virtual ~IAIModel() = default;

    /**
     * @brief Process the project to generate ML insights (e.g., transcription, scene detection).
     * @param projectPath The path to the active `.ssa` recording project bundle.
     * @return true if successful, false otherwise.
     */
    virtual bool process(const std::string& projectPath) = 0;
};

} // namespace ssa::ai
