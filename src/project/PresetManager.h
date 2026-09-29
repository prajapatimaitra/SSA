#pragma once

#include "project/ProjectMetadata.h"
#include <QString>
#include <QList>
#include <string>

namespace ssa::project {

struct StyleConfig {
    std::string cursorColor = "#ff3333";
    double cursorScale = 1.0;
    std::string backgroundColor = "#000000";
    std::string aspectRatio = "16:9"; // Adding aspect ratio here as requested
};

struct ExportConfig {
    std::string codec = "h264"; // h264, hevc, gif
    std::string aspectRatio = "16:9";
    std::string resolution = "1080p"; // 4K, 1440p, 1080p, 720p
    std::string quality = "High"; // High, Medium, Low
    bool hardwareAccel = true;
};

struct RecordingConfig {
    bool isCustomRegion = false;
    int regionX = 0;
    int regionY = 0;
    int regionWidth = 0;
    int regionHeight = 0;
    int fps = 30; // 30 or 60
    std::string captureResolution = "Original";
    int audioSource = 2; // 0 = Mic, 1 = System, 2 = Both
};

struct Preset {
    QString name;
    StyleConfig style;
    ExportConfig exportCfg;
    RecordingConfig recording;
};

class PresetManager {
public:
    PresetManager();
    ~PresetManager() = default;

    // Save visual styling properties from metadata into a template
    bool savePreset(const Preset& preset);
    
    // Read a preset from disk
    bool loadPreset(const QString& presetName, Preset& outPreset);
    
    // Helper to apply only the visual styling to an active project
    bool applyStyleToProject(ProjectMetadata& metadata, const Preset& preset);
    
    // Get a list of all saved template names
    QList<QString> getAvailablePresets() const;

private:
    QString m_presetsDir;
};

} // namespace ssa::project
