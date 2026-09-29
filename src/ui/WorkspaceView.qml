import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 6.0
import "components"

Item {
    id: root

    signal openRecording(string path)
    signal openScreenshot(string path)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 24

        // Top Header Glass Panel
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: headerFlow.implicitHeight + 24
            radius: 16
            color: "#c00f172a" // Translucent dark slate glass
            border.color: "#38bdf8"
            border.width: 1.5

            Flow {
                id: headerFlow
                anchors.fill: parent
                anchors.margins: 12
                spacing: 14

                RowLayout {
                    spacing: 10

                    Text {
                        text: "SSA Workspace"
                        color: "#f8fafc"
                        font.pixelSize: 26
                        font.bold: true
                    }

                    Rectangle {
                        height: 22
                        width: 70
                        radius: 11
                        color: "#1e293b"
                        border.color: "#38bdf8"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "PRO ENGINE"
                            color: "#38bdf8"
                            font.pixelSize: 9
                            font.bold: true
                        }
                    }
                }

                UserProfileBadge {
                    id: userProfileBadge
                }

                // Admin Dashboard Button
                Rectangle {
                    width: adminText.implicitWidth + 28
                    height: 38
                    radius: 10
                    visible: typeof authManager !== "undefined" && authManager.isAdmin
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#7c3aed" }
                        GradientStop { position: 1.0; color: "#8b5cf6" }
                    }
                    border.color: adminMouse.containsMouse ? "#c084fc" : "transparent"
                    border.width: 1

                    Text {
                        id: adminText
                        anchors.centerIn: parent
                        text: "📊 Admin Dashboard"
                        color: "#ffffff"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    MouseArea {
                        id: adminMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof adminDashboardModal !== "undefined") {
                                adminDashboardModal.openDashboard();
                            }
                        }
                    }
                }

                // Search Bar
                TextField {
                    id: searchField
                    placeholderText: "🔍 Search media..."
                    placeholderTextColor: "#64748b"
                    width: 220
                    color: "#f8fafc"
                    font.pixelSize: 13
                    background: Rectangle {
                        color: "#1e293b"
                        radius: 10
                        border.color: searchField.activeFocus ? "#38bdf8" : "#334155"
                        border.width: 1.5
                    }
                    onTextChanged: {
                        if (libraryFilterModel) {
                            libraryFilterModel.searchQuery = text
                        }
                    }
                }
                
                // Filter ComboBox
                ComboBox {
                    id: filterCombo
                    width: 110
                    model: ["All", "Recording", "Screenshot"]
                    onCurrentTextChanged: {
                        if (libraryFilterModel) {
                            if (currentText === "All") libraryFilterModel.typeFilter = ""
                            else libraryFilterModel.typeFilter = currentText.toLowerCase()
                        }
                    }
                }

                // Camera Selector
                MediaDevices { id: mediaDevices }
                ComboBox {
                    id: cameraCombo
                    width: 140
                    model: mediaDevices.videoInputs
                    textRole: "description"
                    displayText: currentIndex !== -1 ? currentText : "No Webcam"
                }

                // Resolution Selector
                ComboBox {
                    id: resolutionCombo
                    width: 110
                    model: ["Native", "4K", "1440p", "1080p", "720p"]
                    currentIndex: 0
                    displayText: currentIndex !== -1 ? currentText : "Resolution"
                }

                // FPS Selector
                ComboBox {
                    id: fpsCombo
                    width: 95
                    model: ["30 FPS", "60 FPS"]
                    currentIndex: 1 // 60 FPS
                    displayText: currentIndex !== -1 ? currentText : "FPS"
                }

                // Audio Selector
                ComboBox {
                    id: audioSourceCombo
                    width: 145
                    model: ["Microphone Only", "System Audio Only", "Both (Mic + System)"]
                    currentIndex: 2 // Both
                    displayText: currentIndex !== -1 ? currentText : "Audio Source"
                }

                // Select Region Button
                Rectangle {
                    width: 125
                    height: 38
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#059669" }
                        GradientStop { position: 1.0; color: "#10b981" }
                    }
                    border.color: regionMouse.containsMouse ? "#6ee7b7" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "🎯 Select Region"
                        color: "#ffffff"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    MouseArea {
                        id: regionMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: regionSelector.show()
                    }
                }

                // Start Fullscreen Recording Button
                Rectangle {
                    width: 145
                    height: 38
                    radius: 10
                    gradient: Gradient {
                        GradientStop { 
                            position: 0.0
                            color: (appState && appState.currentState === 1) ? "#dc2626" : "#0284c7"
                        }
                        GradientStop { 
                            position: 1.0
                            color: (appState && appState.currentState === 1) ? "#ef4444" : "#38bdf8"
                        }
                    }
                    border.color: recMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: (appState && appState.currentState === 1) ? "⏹ Stop Capture" : "⏺ Start Capture"
                        color: "#ffffff"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    MouseArea {
                        id: recMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (appState && appState.currentState === 1) {
                                captureController.stopCapture()
                            } else {
                                var cameraId = "";
                                if (cameraCombo.currentIndex !== -1) {
                                    cameraId = mediaDevices.videoInputs[cameraCombo.currentIndex].id;
                                }
                                var fps = (fpsCombo.currentIndex === 0) ? 30 : 60;
                                var res = resolutionCombo.currentText;
                                if (res === "Native") res = "original";
                                var audioSrc = audioSourceCombo.currentIndex;
                                captureController.startCapture("Default", "display_0", cameraId, fps, res, audioSrc, 0, 0, 0, 0)
                                appState.transitionTo(1) // RECORDING
                            }
                        }
                    }
                }

                // Region Selector Component
                RegionSelector {
                    id: regionSelector
                    onRegionSelected: (rx, ry, rw, rh) => {
                        var cameraId = "";
                        if (cameraCombo.currentIndex !== -1) {
                            cameraId = mediaDevices.videoInputs[cameraCombo.currentIndex].id;
                        }
                        var fps = (fpsCombo.currentIndex === 0) ? 30 : 60;
                        var res = resolutionCombo.currentText;
                        if (res === "Native") res = "original";
                        var audioSrc = audioSourceCombo.currentIndex;
                        
                        captureController.startCapture("Default", "display_0", cameraId, fps, res, audioSrc, rx, ry, rw, rh)
                        appState.transitionTo(1) // RECORDING
                    }
                    onCanceled: {
                        console.log("Region selection canceled")
                    }
                }
            }
        }
        
        // Recording indicator strip (Glassmorphic Red Pill)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "#d9ef4444"
            visible: (appState && appState.currentState === 1)
            radius: 10
            border.color: "#fca5a5"
            border.width: 1
            
            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                Rectangle {
                    width: 10; height: 10; radius: 5; color: "#ffffff"
                    SequentialAnimation on opacity {
                        running: (appState && appState.currentState === 1)
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.2; duration: 600 }
                        NumberAnimation { from: 0.2; to: 1.0; duration: 600 }
                    }
                }
                Text {
                    text: "RECORDING IN PROGRESS — Press Stop Capture when done"
                    color: "#ffffff"
                    font.pixelSize: 13
                    font.bold: true
                }
            }
        }

        // Main Grid Container
        GridView {
            id: mediaGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            cellWidth: 275
            cellHeight: 225
            
            model: libraryFilterModel

            delegate: MediaCard {
                mediaId: model.id
                type: model.type
                path: model.path
                timestamp: model.timestamp
                thumbnailPath: model.thumbnailPath
                durationMs: model.durationMs
                
                onClicked: {
                    if (type === "recording") {
                        root.openRecording(path)
                    } else if (type === "screenshot") {
                        root.openScreenshot(path)
                    }
                }
            }
            
            // Glassmorphic Empty State Container
            Rectangle {
                anchors.centerIn: parent
                width: 320
                height: 160
                radius: 16
                color: "#900f172a"
                border.color: "#1e293b"
                visible: mediaGrid.count === 0

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10
                    Text {
                        text: "🎬"
                        font.pixelSize: 32
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "No Media Files Yet"
                        color: "#f8fafc"
                        font.pixelSize: 16
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "Start a capture or drop video files here"
                        color: "#94a3b8"
                        font.pixelSize: 12
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    // Drag and Drop Area for importing media
    DropArea {
        id: dropArea
        anchors.fill: parent
        
        onDropped: (drop) => {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; ++i) {
                    var url = drop.urls[i].toString()
                    var lowerUrl = url.toLowerCase()
                    if (lowerUrl.endsWith(".mp4") || lowerUrl.endsWith(".mov") || lowerUrl.endsWith(".png") || lowerUrl.endsWith(".jpg")) {
                        if (captureController) {
                            captureController.importExternalMedia(url)
                        }
                        break;
                    }
                }
            }
        }
        
        Rectangle {
            anchors.fill: parent
            color: "#cc070b19"
            visible: dropArea.containsDrag
            
            Rectangle {
                anchors.centerIn: parent
                width: 420
                height: 180
                color: "#e60f172a"
                radius: 16
                border.color: "#38bdf8"
                border.width: 2.5
                
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    Text {
                        text: "📥"
                        font.pixelSize: 40
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "Drop media file here to import"
                        color: "#f8fafc"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "Supports MP4, MOV, PNG, JPG"
                        color: "#38bdf8"
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }
}
