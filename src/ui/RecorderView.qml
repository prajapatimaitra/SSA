import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 6.0

Item {
    id: root

    Rectangle {
        anchors.centerIn: parent
        width: 360
        implicitHeight: cardColumn.implicitHeight + 48
        radius: 20
        color: "#c00f172a" // Translucent dark glass
        border.color: "#38bdf8"
        border.width: 1.5

        ColumnLayout {
            id: cardColumn
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16

            Text {
                text: "SSA Core Engine"
                color: "#f8fafc"
                font.pixelSize: 22
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Status: " + (appState ? (appState.currentState === 1 ? "⏺ Recording" : "⏸ Idle") : "Unknown")
                color: (appState && appState.currentState === 1) ? "#ef4444" : "#38bdf8"
                font.pixelSize: 14
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            MediaDevices { id: mediaDevices }

            ComboBox {
                id: presetCombo
                Layout.fillWidth: true
                model: editorController ? editorController.getAvailablePresets() : ["Default"]
                displayText: currentIndex !== -1 ? currentText : "Select Preset"
            }

            ComboBox {
                id: fpsCombo
                Layout.fillWidth: true
                model: ["30 FPS", "60 FPS"]
                currentIndex: 1 // Default to 60 FPS
                displayText: currentIndex !== -1 ? currentText : "FPS"
            }

            ComboBox {
                id: resolutionCombo
                Layout.fillWidth: true
                model: ["Native", "4K", "1440p", "1080p", "720p"]
                currentIndex: 0
                displayText: currentIndex !== -1 ? currentText : "Resolution"
            }

            ComboBox {
                id: audioSourceCombo
                Layout.fillWidth: true
                model: ["Microphone Only", "System Audio Only", "Both (Mic + System)"]
                currentIndex: 2 // Both
                displayText: currentIndex !== -1 ? currentText : "Audio Source"
            }

            ComboBox {
                id: cameraCombo
                Layout.fillWidth: true
                model: mediaDevices.videoInputs
                textRole: "description"
                displayText: currentIndex !== -1 ? currentText : "No Webcam"
            }

            Rectangle {
                Layout.fillWidth: true
                height: 42
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
                    font.pixelSize: 14
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
                            var presetName = "Default";
                            if (presetCombo.currentIndex !== -1) {
                                presetName = presetCombo.currentText;
                            }
                            var fps = (fpsCombo.currentIndex === 0) ? 30 : 60;
                            var res = resolutionCombo.currentText;
                            if (res === "Native") res = "original";
                            var audioSrc = audioSourceCombo.currentIndex;
                            captureController.startCapture(presetName, "display_0", cameraId, fps, res, audioSrc)
                            appState.transitionTo(1) // RECORDING
                        }
                    }
                }
            }
        }
    }
}
