#pragma once

#include "IAIModel.h"

namespace ssa::ai {

class MockTranscriptionModel : public IAIModel {
public:
    MockTranscriptionModel() = default;
    
    bool process(const std::string& projectPath) override;
};

} // namespace ssa::ai
