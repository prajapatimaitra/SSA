#include "PresetManager.h"
#include "core/logging/Logger.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

namespace ssa::project {

PresetManager::PresetManager() {
    m_presetsDir = "/Users/maitraprajapati/.gemini/antigravity-ide/presets";
    QDir().mkpath(m_presetsDir);
}

bool PresetManager::savePreset(const Preset& preset) {
    QJsonObject rootObj;
    
    QJsonObject styleObj;
    styleObj["cursorColor"] = QString::fromStdString(preset.style.cursorColor);
    styleObj["cursorScale"] = preset.style.cursorScale;
    styleObj["backgroundColor"] = QString::fromStdString(preset.style.backgroundColor);
    styleObj["aspectRatio"] = QString::fromStdString(preset.style.aspectRatio);
    rootObj["StyleConfig"] = styleObj;
    
    QJsonObject exportObj;
    exportObj["codec"] = QString::fromStdString(preset.exportCfg.codec);
    exportObj["aspectRatio"] = QString::fromStdString(preset.exportCfg.aspectRatio);
    exportObj["resolution"] = QString::fromStdString(preset.exportCfg.resolution);
    exportObj["quality"] = QString::fromStdString(preset.exportCfg.quality);
    exportObj["hardwareAccel"] = preset.exportCfg.hardwareAccel;
    rootObj["ExportConfig"] = exportObj;
    
    QJsonObject recObj;
    recObj["isCustomRegion"] = preset.recording.isCustomRegion;
    recObj["regionX"] = preset.recording.regionX;
    recObj["regionY"] = preset.recording.regionY;
    recObj["regionWidth"] = preset.recording.regionWidth;
    recObj["regionHeight"] = preset.recording.regionHeight;
    recObj["fps"] = preset.recording.fps;
    recObj["captureResolution"] = QString::fromStdString(preset.recording.captureResolution);
    recObj["audioSource"] = preset.recording.audioSource;
    rootObj["RecordingConfig"] = recObj;
    
    QJsonDocument doc(rootObj);
    QString path = m_presetsDir + "/" + preset.name + ".json";
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        core::logging::Logger::error("Failed to write preset: " + path.toStdString());
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    core::logging::Logger::info("Successfully saved preset: " + preset.name.toStdString());
    return true;
}

bool PresetManager::loadPreset(const QString& presetName, Preset& outPreset) {
    QString path = m_presetsDir + "/" + presetName + ".json";
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        outPreset.name = presetName;
        savePreset(outPreset);
        return true;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) return false;
    QJsonObject rootObj = doc.object();
    outPreset.name = presetName;
    
    if (rootObj.contains("StyleConfig")) {
        QJsonObject sObj = rootObj["StyleConfig"].toObject();
        if (sObj.contains("cursorColor")) outPreset.style.cursorColor = sObj["cursorColor"].toString().toStdString();
        if (sObj.contains("cursorScale")) outPreset.style.cursorScale = sObj["cursorScale"].toDouble();
        if (sObj.contains("backgroundColor")) outPreset.style.backgroundColor = sObj["backgroundColor"].toString().toStdString();
        if (sObj.contains("aspectRatio")) outPreset.style.aspectRatio = sObj["aspectRatio"].toString().toStdString();
    }
    
    if (rootObj.contains("ExportConfig")) {
        QJsonObject eObj = rootObj["ExportConfig"].toObject();
        if (eObj.contains("codec")) outPreset.exportCfg.codec = eObj["codec"].toString().toStdString();
        if (eObj.contains("aspectRatio")) outPreset.exportCfg.aspectRatio = eObj["aspectRatio"].toString().toStdString();
        if (eObj.contains("resolution")) outPreset.exportCfg.resolution = eObj["resolution"].toString().toStdString();
        if (eObj.contains("quality")) outPreset.exportCfg.quality = eObj["quality"].toString().toStdString();
        if (eObj.contains("hardwareAccel")) outPreset.exportCfg.hardwareAccel = eObj["hardwareAccel"].toBool();
    }
    
    if (rootObj.contains("RecordingConfig")) {
        QJsonObject rObj = rootObj["RecordingConfig"].toObject();
        if (rObj.contains("isCustomRegion")) outPreset.recording.isCustomRegion = rObj["isCustomRegion"].toBool();
        if (rObj.contains("regionX")) outPreset.recording.regionX = rObj["regionX"].toInt();
        if (rObj.contains("regionY")) outPreset.recording.regionY = rObj["regionY"].toInt();
        if (rObj.contains("regionWidth")) outPreset.recording.regionWidth = rObj["regionWidth"].toInt();
        if (rObj.contains("regionHeight")) outPreset.recording.regionHeight = rObj["regionHeight"].toInt();
        if (rObj.contains("fps")) outPreset.recording.fps = rObj["fps"].toInt();
        if (rObj.contains("captureResolution")) outPreset.recording.captureResolution = rObj["captureResolution"].toString().toStdString();
        if (rObj.contains("audioSource")) outPreset.recording.audioSource = rObj["audioSource"].toInt();
    }
    
    core::logging::Logger::info("Successfully loaded preset: " + presetName.toStdString());
    return true;
}

bool PresetManager::applyStyleToProject(ProjectMetadata& metadata, const Preset& preset) {
    metadata.cursorColor = preset.style.cursorColor;
    metadata.cursorScale = preset.style.cursorScale;
    metadata.backgroundColor = preset.style.backgroundColor;
    return true;
}

QList<QString> PresetManager::getAvailablePresets() const {
    QList<QString> list;
    QDir dir(m_presetsDir);
    
    QStringList filters;
    filters << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fi : files) {
        list.append(fi.baseName());
    }
    
    return list;
}

} // namespace ssa::project

