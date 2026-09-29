import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import "components"

Window {
    id: mainWindow
    width: 1024
    height: 768
    visible: true
    title: qsTr("SSA - Screen Studio Alternative")
    color: "#070b19" // Midnight dark baseline

    Component.onCompleted: {
        if (typeof nativeIntegration !== "undefined" && nativeIntegration.setAppCaptureProtected) {
            nativeIntegration.setAppCaptureProtected(true)
            nativeIntegration.setWindowCaptureProtected(mainWindow, true)
        }
    }

    // Dynamic Glassmorphism Background Canvas with Animated Light Mesh
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#060a17" }
            GradientStop { position: 0.5; color: "#091126" }
            GradientStop { position: 1.0; color: "#0d1527" }
        }

        // Animated Ambient Neon Light Spheres
        Rectangle {
            id: orb1
            width: 450; height: 450; radius: 225
            x: parent.width * 0.05
            y: -120
            color: "#2500f2fe" // Vivid Cyan Glow
            
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation { to: mainWindow.width * 0.25; duration: 12000; easing.type: Easing.InOutSine }
                NumberAnimation { to: mainWindow.width * 0.05; duration: 12000; easing.type: Easing.InOutSine }
            }
            SequentialAnimation on y {
                loops: Animation.Infinite
                NumberAnimation { to: 80; duration: 9000; easing.type: Easing.InOutSine }
                NumberAnimation { to: -120; duration: 9000; easing.type: Easing.InOutSine }
            }
        }

        Rectangle {
            id: orb2
            width: 550; height: 550; radius: 275
            x: parent.width * 0.6
            y: parent.height * 0.4
            color: "#207c3aed" // Deep Violet Glow

            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation { to: mainWindow.width * 0.45; duration: 15000; easing.type: Easing.InOutSine }
                NumberAnimation { to: mainWindow.width * 0.6; duration: 15000; easing.type: Easing.InOutSine }
            }
            SequentialAnimation on y {
                loops: Animation.Infinite
                NumberAnimation { to: mainWindow.height * 0.25; duration: 11000; easing.type: Easing.InOutSine }
                NumberAnimation { to: mainWindow.height * 0.4; duration: 11000; easing.type: Easing.InOutSine }
            }
        }

        Rectangle {
            id: orb3
            width: 380; height: 380; radius: 190
            x: parent.width * 0.35
            y: parent.height * 0.65
            color: "#18ec4899" // Pink Magenta Accent Glow

            SequentialAnimation on scale {
                loops: Animation.Infinite
                NumberAnimation { to: 1.25; duration: 8000; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 0.9; duration: 8000; easing.type: Easing.InOutQuad }
            }
        }
    }

    Connections {
        target: captureController
        function onCaptureFinished(projectPath) {
            console.log("Capture finished, loading project: " + projectPath)
            editorController.loadProject(projectPath)
            appState.transitionTo(4) // READY
        }
        function onScreenshotCaptured(imagePath) {
            console.log("Screenshot captured: " + imagePath)
            screenshotEditor.screenshotPath = imagePath
            screenshotEditor.visible = true
        }
    }

    StackLayout {
        anchors.fill: parent
        // AppState: 4=READY (Editor)
        currentIndex: (appState && appState.currentState === 4) ? 1 : 0

        WorkspaceView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            onOpenRecording: function(path) {
                console.log("Loading project: " + path)
                editorController.loadProject(path)
                appState.transitionTo(4) // READY (Editor)
            }
            
            onOpenScreenshot: function(path) {
                console.log("Opening screenshot: " + path)
                screenshotEditor.screenshotPath = path
                screenshotEditor.visible = true
            }
        }

        EditorView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
    
    ScreenshotEditor {
        id: screenshotEditor
        anchors.fill: parent
        visible: false
        z: 100 // Ensure it floats above everything else
    }
    
    CommandPalette {
        id: commandPalette
        x: (parent.width - width) / 2
        y: (parent.height - height) / 3
        z: 200
        
        onTriggerRegionSelector: {
            appState.transitionTo(0);
        }
    }

    ExportQuotaModal {
        id: quotaModal
        z: 350
        onOpenEmailAuth: {
            emailAuthModal.openModal();
        }
    }

    EmailAuthModal {
        id: emailAuthModal
        z: 360
    }

    AdminDashboard {
        id: adminDashboardModal
        z: 400
    }

    Shortcut {
        sequence: "Ctrl+Shift+A"
        enabled: typeof authManager !== "undefined" && authManager.isAdmin
        onActivated: {
            if (authManager.isAdmin) {
                adminDashboardModal.openDashboard();
            }
        }
    }

    Connections {
        target: typeof quotaManager !== "undefined" ? quotaManager : null
        function onExportBlocked(title, message) {
            quotaModal.showModal(title, message);
        }
    }
    
    Shortcut {
        sequence: "Ctrl+K"
        onActivated: {
            if (commandPalette.opened) {
                commandPalette.close()
            } else {
                commandPalette.open()
            }
        }
    }
    
    // Global Render Queue Overlay (Glassmorphic Draggable Toast Card)
    Rectangle {
        id: renderQueueOverlay
        x: 20
        y: parent.height - height - 20
        width: 340
        height: 72
        radius: 14
        color: "#d90f172a" // Translucent dark glass
        border.color: "#38bdf8"
        border.width: 1.5
        visible: typeof exportQueueManager !== "undefined" && exportQueueManager.isExporting
        z: 300 // Floating overlay over editor & workspace
        
        MouseArea {
            id: dragArea
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            
            property point clickPos: "0,0"

            onPressed: function(mouse) {
                clickPos = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: function(mouse) {
                var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                renderQueueOverlay.x = Math.max(0, Math.min(mainWindow.width - renderQueueOverlay.width, renderQueueOverlay.x + delta.x))
                renderQueueOverlay.y = Math.max(0, Math.min(mainWindow.height - renderQueueOverlay.height, renderQueueOverlay.y + delta.y))
            }
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                
                Text {
                    text: "⚡"
                    font.pixelSize: 14
                }
                
                Text {
                    text: {
                        if (typeof exportQueueManager === "undefined") return "";
                        var pct = Math.round(exportQueueManager.currentProgress * 100);
                        var remaining = exportQueueManager.queueSize - 1;
                        var label = "Rendering: " + exportQueueManager.currentProjectName + " (" + pct + "%)";
                        if (remaining > 0) {
                            label += " + " + remaining + " queued";
                        }
                        return label;
                    }
                    color: "#f8fafc"
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                // Drag indicator icon
                Text {
                    text: "⠿"
                    color: "#64748b"
                    font.pixelSize: 14
                }

                // Cancel Export Button
                Rectangle {
                    width: 22
                    height: 22
                    radius: 11
                    color: cancelMouse.containsMouse ? "#ef4444" : "#1e293b"
                    border.color: "#334155"
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: cancelMouse.containsMouse ? "#ffffff" : "#94a3b8"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof exportQueueManager !== "undefined") {
                                exportQueueManager.cancelCurrentExport();
                            }
                        }
                    }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 6
                radius: 3
                color: "#1e293b"
                
                Rectangle {
                    height: parent.height
                    width: parent.width * (typeof exportQueueManager !== "undefined" ? Math.max(0.02, exportQueueManager.currentProgress) : 0)
                    radius: 3
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#38bdf8" }
                        GradientStop { position: 1.0; color: "#8b5cf6" }
                    }
                }
            }
        }
    }
}
