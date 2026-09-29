import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 255
    height: 205
    radius: 14
    color: "#b30f172a" // Translucent dark glass fill
    border.color: mouseArea.containsMouse ? "#38bdf8" : "#40ffffff"
    border.width: mouseArea.containsMouse ? 1.5 : 1
    clip: true

    y: mouseArea.containsMouse ? -4 : 0
    scale: mouseArea.containsMouse ? 1.02 : 1.0
    Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }

    // Specular Glass Rim Light (Top edge reflection)
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: "#60ffffff"
    }

    property int mediaId: -1
    property string type: "recording"
    property string path: ""
    property var timestamp: 0
    property string thumbnailPath: ""
    property int durationMs: 0
    
    signal clicked()

    // Thumbnail Image Container
    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: 10
        color: "#090d16"
        clip: true

        Image {
            id: thumbnail
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            source: root.thumbnailPath !== "" ? "file://" + root.thumbnailPath : ""
            
            // Fallback placeholder for videos without thumbnails
            Rectangle {
                anchors.fill: parent
                color: "#0b1329"
                visible: thumbnail.status !== Image.Ready
                
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text {
                        text: root.type === "recording" ? "🎬" : "📸"
                        font.pixelSize: 32
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: root.type === "recording" ? "Video Capture" : "Screenshot"
                        color: "#64748b"
                        font.pixelSize: 12
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        // Gradient Overlay for text readability
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 70
            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 1.0; color: "#f0070b19" }
            }
        }

        // Metadata Overlay
        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                
                // Pill Type Badge
                Rectangle {
                    height: 20
                    width: root.type === "recording" ? 78 : 82
                    radius: 10
                    gradient: Gradient {
                        GradientStop { 
                            position: 0.0
                            color: root.type === "recording" ? "#0284c7" : "#7c3aed"
                        }
                        GradientStop { 
                            position: 1.0
                            color: root.type === "recording" ? "#38bdf8" : "#c084fc"
                        }
                    }
                    
                    Text {
                        anchors.centerIn: parent
                        text: root.type === "recording" ? "⏺ Video" : "📸 Snap"
                        color: "#ffffff"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Item { Layout.fillWidth: true }
                
                Text {
                    text: root.type === "recording" ? (Math.round(root.durationMs / 1000) + "s") : ""
                    color: "#f8fafc"
                    font.pixelSize: 11
                    font.bold: true
                    visible: root.type === "recording"
                }
            }
            
            Text {
                text: new Date(root.timestamp).toLocaleString(Qt.locale(), "MMM d, yyyy - h:mm AP")
                color: "#94a3b8"
                font.pixelSize: 10
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: Qt.PointingHandCursor
        
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                contextMenu.popup()
            } else {
                root.clicked()
            }
        }
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: "Open Editor"
            onTriggered: root.clicked()
        }
        
        MenuItem {
            text: "Reveal in Finder"
            onTriggered: {
                if (typeof nativeIntegration !== "undefined") {
                    nativeIntegration.revealInOS(root.path)
                }
            }
        }
        
        MenuItem {
            text: "Share..."
            onTriggered: {
                if (typeof nativeIntegration !== "undefined") {
                    nativeIntegration.shareFile(root.path)
                }
            }
        }
        
        MenuSeparator {}
        
        MenuItem {
            text: "Delete"
            contentItem: Text {
                text: parent.text
                color: "#ef4444"
                font.bold: true
            }
            onTriggered: {
                if (captureController) {
                    captureController.deleteProject(root.path);
                }
            }
        }
    }
    
    // Action Overlay (visible on hover) - Frosted Glass Bar
    RowLayout {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 6
        visible: mouseArea.containsMouse && root.type === "recording"
        
        Rectangle {
            width: 30
            height: 30
            radius: 15
            color: "#e60f172a"
            border.color: dupMouseArea.containsMouse ? "#38bdf8" : "#334155"
            border.width: 1
            
            Text {
                anchors.centerIn: parent
                text: "📋"
                font.pixelSize: 13
            }
            
            MouseArea {
                id: dupMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (captureController) {
                        captureController.duplicateProject(root.path);
                    }
                }
            }
        }
        
        Rectangle {
            width: 30
            height: 30
            radius: 15
            color: "#e6ef4444"
            border.color: delMouseArea.containsMouse ? "#ffffff" : "transparent"
            border.width: 1
            
            Text {
                anchors.centerIn: parent
                text: "🗑️"
                font.pixelSize: 13
            }
            
            MouseArea {
                id: delMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (captureController) {
                        captureController.deleteProject(root.path);
                    }
                }
            }
        }
    }
}
