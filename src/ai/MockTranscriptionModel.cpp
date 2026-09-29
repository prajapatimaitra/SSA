#include "MockTranscriptionModel.h"
#include "core/logging/Logger.h"
#include <fstream>
#include <thread>
#include <chrono>

namespace ssa::ai {

bool MockTranscriptionModel::process(const std::string& projectPath) {
    core::logging::Logger::info("MockTranscriptionModel: Processing " + projectPath);
    
    // Simulate AI model processing time
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::string vttPath = projectPath + "/transcription.vtt";
    std::ofstream vttFile(vttPath);
    if (!vttFile.is_open()) {
        core::logging::Logger::error("MockTranscriptionModel: Failed to open VTT file for writing.");
        return false;
    }
    
    // Write WebVTT header
    vttFile << "WEBVTT\n\n";
    
    // Write some mock subtitles
    vttFile << "00:00:00.000 --> 00:00:02.000\n";
    vttFile << "Hello! This is a mock AI transcription.\n\n";
    
    vttFile << "00:00:02.000 --> 00:00:05.000\n";
    vttFile << "It demonstrates the Phase 16 subtitle capability.\n\n";
    
    vttFile << "00:00:05.000 --> 00:00:08.000\n";
    vttFile << "The architecture is built to swap this out with Whisper.cpp easily.\n\n";
    
    vttFile << "00:00:08.000 --> 00:00:15.000\n";
    vttFile << "Hope you like this quick placeholder!\n\n";
    
    vttFile.close();
    
    core::logging::Logger::info("MockTranscriptionModel: VTT file generated successfully at " + vttPath);
    return true;
}

} // namespace ssa::ai
