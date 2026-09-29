import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: root
    color: "#121214"
    border.color: "#27272a"
    border.width: 1

    signal assetDoubleClicked(int assetId, string filePath, string mediaType)

    property bool isGridView: true

    FileDialog {
        id: importFileDialog
        title: "Import Media Files"
        nameFilters: [
            "Media Files (*.mp4 *.mov *.mkv *.avi *.webm *.mp3 *.wav *.m4a *.png *.jpg *.jpeg *.webp)",
            "Video Files (*.mp4 *.mov *.mkv *.avi *.webm)",
            "Audio Files (*.mp3 *.wav *.m4a *.aac *.flac)",
            "Image Files (*.png *.jpg *.jpeg *.webp *.svg)",
            "All Files (*)"
        ]
        onAccepted: {
            if (typeof projectBinManager !== "undefined") {
                projectBinManager.importMediaFile(importFileDialog.selectedFile.toString(), projectBinManager.selectedBinId)
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            color: "#1c1c1f"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                // Search Box
                Rectangle {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 260
                    implicitHeight: 28
                    color: "#27272a"
                    radius: 6

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 6

                        Text {
                            text: "🔍"
                            font.pixelSize: 11
                            color: "#71717a"
                        }

                        TextInput {
                            id: searchInput
                            Layout.fillWidth: true
                            color: "#ffffff"
                            font.pixelSize: 12
                            clip: true
                            onTextChanged: {
                                if (typeof projectBinManager !== "undefined") {
                                    projectBinManager.setSearchQuery(text)
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Grid / List Toggle Button
                Button {
                    text: root.isGridView ? "≡ List" : "⊞ Grid"
                    font.pixelSize: 11
                    flat: true
                    onClicked: root.isGridView = !root.isGridView
                }

                // Import Button
                Button {
                    text: "+ Import Media"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    background: Rectangle {
                        color: parent.hovered ? "#4338ca" : "#4f46e5"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: importFileDialog.open()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#27272a"
        }

        // Empty state indicator
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: assetModel.count === 0

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: "📁"
                    font.pixelSize: 42
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "No Media in Selected Bin"
                    color: "#a1a1aa"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Drag and drop media files here or click '+ Import Media'"
                    color: "#71717a"
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Main Grid View
        GridView {
            id: assetGridView
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.isGridView && assetModel.count > 0
            clip: true
            cellWidth: 160
            cellHeight: 140

            model: typeof projectBinManager !== "undefined" ? projectBinManager.assetModel : null
            id: assetModel

            delegate: Item {
                width: assetGridView.cellWidth
                height: assetGridView.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 6
                    color: itemMouseArea.containsMouse ? "#27272a" : "#18181b"
                    border.color: itemMouseArea.containsMouse ? "#6366f1" : "#27272a"
                    border.width: 1
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 4

                        // Thumbnail Container
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "#09090b"
                            radius: 4
                            clip: true

                            Image {
                                anchors.fill: parent
                                source: model.thumbnailPath ? "file://" + model.thumbnailPath : ""
                                fillMode: Image.PreserveAspectCrop
                                visible: model.thumbnailPath && model.thumbnailPath.length > 0
                            }

                            // Fallback Icon when no thumbnail
                            Text {
                                anchors.centerIn: parent
                                visible: !model.thumbnailPath || model.thumbnailPath.length === 0
                                text: model.mediaType === "audio" ? "🎵" : (model.mediaType === "image" ? "🖼️" : "🎬")
                                font.pixelSize: 28
                            }

                            // Duration Badge
                            Rectangle {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.margins: 4
                                implicitWidth: durText.implicitWidth + 8
                                implicitHeight: 16
                                color: "#cc000000"
                                radius: 3

                                Text {
                                    id: durText
                                    anchors.centerIn: parent
                                    text: model.formattedDuration
                                    color: "#f4f4f5"
                                    font.pixelSize: 9
                                    font.weight: Font.Medium
                                }
                            }

                            // Media Type Tag
                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: 4
                                implicitWidth: typeText.implicitWidth + 6
                                implicitHeight: 14
                                color: model.mediaType === "audio" ? "#3b82f6" : (model.mediaType === "image" ? "#10b981" : "#6366f1")
                                radius: 3

                                Text {
                                    id: typeText
                                    anchors.centerIn: parent
                                    text: model.mediaType.toUpperCase()
                                    color: "#ffffff"
                                    font.pixelSize: 8
                                    font.weight: Font.Bold
                                }
                            }
                        }

                        // Asset Name
                        Text {
                            text: model.name
                            color: "#e4e4e7"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: itemMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onDoubleClicked: {
                            root.assetDoubleClicked(model.assetId, model.filePath, model.mediaType)
                        }

                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton) {
                                assetContextMenu.assetId = model.assetId
                                assetContextMenu.popup()
                            }
                        }
                    }
                }
            }
        }
    }

    // Drag and Drop support
    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.hasUrls && typeof projectBinManager !== "undefined") {
                for (var i = 0; i < drop.urls.length; ++i) {
                    projectBinManager.importMediaFile(drop.urls[i], projectBinManager.selectedBinId)
                }
            }
        }
    }

    // Context menu for assets
    Menu {
        id: assetContextMenu
        property int assetId: -1

        MenuItem {
            text: "Move to Master Bin"
            onTriggered: {
                if (typeof projectBinManager !== "undefined" && assetContextMenu.assetId > 0) {
                    projectBinManager.moveAssetToBin(assetContextMenu.assetId, 1)
                }
            }
        }
        MenuItem {
            text: "Delete Asset"
            onTriggered: {
                if (typeof projectBinManager !== "undefined" && assetContextMenu.assetId > 0) {
                    projectBinManager.removeAsset(assetContextMenu.assetId)
                }
            }
        }
    }
}
