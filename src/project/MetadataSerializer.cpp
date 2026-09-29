#include "project/MetadataSerializer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

namespace ssa::project {

QString MetadataSerializer::serialize(const ProjectMetadata& metadata) {
    QJsonObject root;
    root["uuid"] = QString::fromStdString(metadata.uuid);
    root["createdAt"] = static_cast<qint64>(metadata.createdAt);
    root["recordingStartTimestamp"] = static_cast<qint64>(metadata.recordingStartTimestamp);
    root["videoWidth"] = metadata.videoWidth;
    root["videoHeight"] = metadata.videoHeight;
    root["cursorColor"] = QString::fromStdString(metadata.cursorColor);
    root["cursorScale"] = metadata.cursorScale;
    root["backgroundColor"] = QString::fromStdString(metadata.backgroundColor);
    root["trimStartTimeMs"] = static_cast<qint64>(metadata.trimStartTimeMs);
    root["trimEndTimeMs"] = static_cast<qint64>(metadata.trimEndTimeMs);

    QJsonArray eventsArray;
    for (const auto& ev : metadata.mouseEvents) {
        QJsonObject eventObj;
        eventObj["timestamp"] = static_cast<qint64>(ev.timestamp);
        eventObj["x"] = ev.x;
        eventObj["y"] = ev.y;
        eventObj["button"] = static_cast<int>(ev.button);
        eventObj["type"] = static_cast<int>(ev.type);
        eventObj["cursor"] = QString::fromStdString(ev.cursorType);
        eventObj["cursorWidth"] = ev.cursorWidth;
        eventObj["cursorHeight"] = ev.cursorHeight;
        eventObj["cursorHotspotX"] = ev.cursorHotspotX;
        eventObj["cursorHotspotY"] = ev.cursorHotspotY;
        eventsArray.append(eventObj);
    }
    root["mouseEvents"] = eventsArray;

    QJsonArray markersArray;
    for (const auto& m : metadata.markers) {
        QJsonObject markerObj;
        markerObj["timestampMs"] = static_cast<qint64>(m.timestampMs);
        markerObj["color"] = QString::fromStdString(m.color);
        markerObj["note"] = QString::fromStdString(m.note);
        markersArray.append(markerObj);
    }
    root["markers"] = markersArray;
    
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

std::optional<ProjectMetadata> MetadataSerializer::deserialize(const QString& jsonString) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return std::nullopt;
    }

    QJsonObject root = doc.object();
    ProjectMetadata meta;
    meta.uuid = root["uuid"].toString().toStdString();
    meta.createdAt = root["createdAt"].toVariant().toULongLong();
    meta.recordingStartTimestamp = root["recordingStartTimestamp"].toVariant().toULongLong();
    meta.videoWidth = root["videoWidth"].toInt();
    meta.videoHeight = root["videoHeight"].toInt();
    if (root.contains("cursorColor")) meta.cursorColor = root["cursorColor"].toString().toStdString();
    if (root.contains("cursorScale")) meta.cursorScale = root["cursorScale"].toDouble();
    if (root.contains("backgroundColor")) meta.backgroundColor = root["backgroundColor"].toString().toStdString();
    if (root.contains("trimStartTimeMs")) meta.trimStartTimeMs = root["trimStartTimeMs"].toVariant().toULongLong();
    if (root.contains("trimEndTimeMs")) meta.trimEndTimeMs = root["trimEndTimeMs"].toVariant().toULongLong();

    QJsonArray eventsArray = root["mouseEvents"].toArray();
    for (const QJsonValue& val : eventsArray) {
        if (!val.isObject()) continue;
        QJsonObject eventObj = val.toObject();
        input::MouseEvent ev;
        ev.timestamp = eventObj["timestamp"].toVariant().toULongLong();
        ev.x = eventObj["x"].toDouble();
        ev.y = eventObj["y"].toDouble();
        ev.button = static_cast<input::MouseButton>(eventObj["button"].toInt());
        ev.type = static_cast<input::MouseEventType>(eventObj["type"].toInt());
        if (eventObj.contains("cursor")) {
            ev.cursorType = eventObj["cursor"].toString().toStdString();
        } else {
            ev.cursorType = "arrow";
        }
        if (eventObj.contains("cursorWidth")) ev.cursorWidth = eventObj["cursorWidth"].toDouble();
        if (eventObj.contains("cursorHeight")) ev.cursorHeight = eventObj["cursorHeight"].toDouble();
        if (eventObj.contains("cursorHotspotX")) ev.cursorHotspotX = eventObj["cursorHotspotX"].toDouble();
        if (eventObj.contains("cursorHotspotY")) ev.cursorHotspotY = eventObj["cursorHotspotY"].toDouble();
        meta.mouseEvents.push_back(ev);
    }

    if (root.contains("markers") && root["markers"].isArray()) {
        QJsonArray markersArray = root["markers"].toArray();
        for (const QJsonValue& val : markersArray) {
            if (!val.isObject()) continue;
            QJsonObject markerObj = val.toObject();
            ProjectMetadata::Marker m;
            m.timestampMs = markerObj["timestampMs"].toVariant().toULongLong();
            m.color = markerObj["color"].toString().toStdString();
            m.note = markerObj["note"].toString().toStdString();
            meta.markers.push_back(m);
        }
    }

    return meta;
}

} // namespace ssa::project
