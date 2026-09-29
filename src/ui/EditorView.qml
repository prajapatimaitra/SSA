import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia
import "components"

Item {
    id: root
    
    function getCursorSource(curType) {
        if (!curType || curType === "") curType = "arrow"
        var base = (typeof appDataUrl !== "undefined" ? appDataUrl : "")
        if (base !== "" && !base.startsWith("file://")) {
            base = "file://" + base
        }
        
        var pngName = "arrow.png"
        if (curType === "arrow") pngName = "arrow.png"
        else if (curType === "pointer" || curType === "pointingHand") pngName = "pointingHand.png"
        else if (curType === "iBeam") pngName = "iBeam.png"
        else if (curType === "iBeamCursorForVerticalLayout") pngName = "iBeamCursorForVerticalLayout.png"
        else if (curType === "closedHand") pngName = "closedHand.png"
        else if (curType === "openHand") pngName = "openHand.png"
        else if (curType === "crosshair") pngName = "crosshair.png"
        else if (curType === "busyButClickable") pngName = "busyButClickable.png"
        else if (curType === "contextualMenu") pngName = "contextualMenu.png"
        else if (curType === "dragCopy") pngName = "dragCopy.png"
        else if (curType === "dragLink") pngName = "dragLink.png"
        else if (curType === "operationNotAllowed") pngName = "operationNotAllowed.png"
        else if (curType === "help") pngName = "help.png"
        else if (curType === "zoomIn") pngName = "zoomIn.png"
        else if (curType === "zoomOut") pngName = "zoomOut.png"
        else if (curType === "resizeLeftRight") pngName = "resizeLeftRight.png"
        else if (curType === "resizeUpDown") pngName = "resizeUpDown.png"
        else if (curType === "resizeDown") pngName = "resizeDown.png"
        else if (curType === "resizeUp") pngName = "resizeUp.png"
        else if (curType === "resizeLeft") pngName = "resizeLeft.png"
        else if (curType === "resizeRight") pngName = "resizeRight.png"
        else pngName = curType + ".png"
        
        return base + "/cursors/" + pngName
    }
    
    // Glassmorphic Top Header Bar
    Rectangle {
        id: topBar
        width: parent.width
        height: Math.max(52, topBarFlow.implicitHeight + 16)
        color: "#b30f172a" // Translucent glossy dark glass
        border.color: "#38bdf8"
        border.width: 1
        z: 10

        Behavior on height { NumberAnimation { duration: 150 } }

        // Specular Rim Light
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: "#60ffffff"
        }
        
        Flow {
            id: topBarFlow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 8
            spacing: 8

            // Back Button
            Rectangle {
                width: 90
                height: 34
                radius: 8
                color: "#1e293b"
                border.color: backMouse.containsMouse ? "#38bdf8" : "#334155"
                border.width: 1.5

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text { text: "◀"; color: "#38bdf8"; font.pixelSize: 11 }
                    Text { text: "Back"; color: "#f8fafc"; font.pixelSize: 12; font.bold: true }
                }

                MouseArea {
                    id: backMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appState.transitionTo(0)
                }
            }

            // Export Controls Group
            RowLayout {
                spacing: 4
                height: 34
                Text { text: "Preset:"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true }
                ComboBox {
                    id: exportPresetCombo
                    implicitWidth: 110
                    implicitHeight: 34
                    model: ["Custom", "YouTube 16:9", "TikTok 9:16"]
                    currentIndex: 0
                    onActivated: {
                        if (index === 1) {
                            aspectRatioCombo.currentIndex = 1; // 16:9
                            if (editorController) editorController.aspectRatio = 1;
                        } else if (index === 2) {
                            aspectRatioCombo.currentIndex = 2; // 9:16
                            if (editorController) editorController.aspectRatio = 2;
                        }
                    }
                }
            }

            RowLayout {
                spacing: 4
                height: 34
                Text { text: "Aspect:"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true }
                ComboBox {
                    id: aspectRatioCombo
                    implicitWidth: 120
                    implicitHeight: 34
                    model: ["Original", "16:9 Landscape", "9:16 Vertical", "1:1 Square"]
                    currentIndex: 0
                    onActivated: {
                        exportPresetCombo.currentIndex = 0; // Set to custom
                        if (editorController) {
                            if (index === 0) editorController.aspectRatio = 0; // RatioOriginal
                            else if (index === 1) editorController.aspectRatio = 1; // Ratio16x9
                            else if (index === 2) editorController.aspectRatio = 2; // Ratio9x16
                            else if (index === 3) editorController.aspectRatio = 3; // Ratio1x1
                        }
                    }
                }
            }

            RowLayout {
                spacing: 4
                height: 34
                Text { text: "Res:"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true }
                ComboBox {
                    id: resolutionCombo
                    implicitWidth: 85
                    implicitHeight: 34
                    model: ["Native", "720p", "1080p", "1440p", "4K"]
                    currentIndex: 0
                }
            }

            // User & Action Group
            UserProfileBadge {
                id: editorUserProfileBadge
            }
            
            // Dashboard Button
            Rectangle {
                width: 95
                height: 34
                radius: 8
                visible: typeof authManager !== "undefined" && authManager.isAdmin
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#7c3aed" }
                    GradientStop { position: 1.0; color: "#8b5cf6" }
                }
                border.color: dashMouse.containsMouse ? "#c084fc" : "transparent"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: "📊 Admin"
                    color: "#ffffff"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: dashMouse
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
            
            // Add to Render Queue Button
            Rectangle {
                width: 120
                height: 34
                radius: 8
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#0284c7" }
                    GradientStop { position: 1.0; color: "#38bdf8" }
                }
                border.color: renderMouse.containsMouse ? "#ffffff" : "transparent"
                border.width: 1.5

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 4
                    Text { text: "⚡"; font.pixelSize: 12 }
                    Text { text: "Render Queue"; color: "#ffffff"; font.pixelSize: 12; font.bold: true }
                }

                MouseArea {
                    id: renderMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("Add to Render Queue clicked")
                        if (editorController && typeof exportQueueManager !== "undefined") {
                            var durationMs = (editorController ? editorController.totalDuration : 60000);
                            var resText = resolutionCombo.currentText;
                            var resHeight = (resText === "4K") ? 2160 : ((resText === "1440p") ? 1440 : 1080);
                            
                            if (typeof quotaManager !== "undefined") {
                                if (!quotaManager.checkExportAllowed(durationMs, resHeight)) {
                                    console.log("Export blocked by quota manager");
                                    return;
                                }
                                quotaManager.recordExport(durationMs);
                            }
                            
                            editorController.saveProject()
                            exportQueueManager.enqueueExport(editorController.projectPath, exportPresetCombo.currentText, resolutionCombo.currentText)
                        }
                    }
                }
            }
            
            // Share Button
            Rectangle {
                width: 70
                height: 34
                radius: 8
                color: "#1e293b"
                border.color: shareMouse.containsMouse ? "#38bdf8" : "#334155"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: "🔗 Share"
                    color: "#f8fafc"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: shareMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (editorController && typeof nativeIntegration !== "undefined") {
                            nativeIntegration.shareFile(editorController.projectPath + "/final_export.mp4")
                        }
                    }
                }
            }
        }
    }
    
    // Properties Panel (Right side - Frosted Glass)
    Rectangle {
        id: propertiesPanel
        width: 260
        anchors.top: topBar.bottom
        anchors.bottom: timelineArea.top
        anchors.right: parent.right
        color: "#b30b1329" // Translucent dark midnight glass
        border.color: "#38bdf8"
        border.width: 1
        z: 5

        // Specular Rim Light
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: "#50ffffff"
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12
            
            Text { 
                text: "⚙️ CANVAS & CURSOR"
                color: "#38bdf8"
                font.pixelSize: 11
                font.bold: true
            }
            
            Text { text: "Background Color / Wallpaper"; color: "#cbd5e1"; font.pixelSize: 12 }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                ComboBox {
                    id: wallpaperCombo
                    Layout.fillWidth: true
                    model: editorController ? ["Custom Color..."].concat(editorController.getAvailableWallpapers()) : ["Custom Color..."]
                    onActivated: {
                        if (editorController && currentText !== "Custom Color...") {
                            editorController.backgroundColor = currentText
                            bgColorField.text = currentText
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                TextField {
                    id: bgColorField
                    text: editorController ? editorController.backgroundColor : "#000000"
                    onEditingFinished: if (editorController) editorController.backgroundColor = text
                    Layout.fillWidth: true
                    color: "#f8fafc"
                    font.pixelSize: 12
                    background: Rectangle {
                        color: "#1e293b"
                        radius: 6
                        border.color: "#334155"
                    }
                }
                Rectangle {
                    width: 55
                    height: 32
                    radius: 6
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#0284c7" }
                        GradientStop { position: 1.0; color: "#38bdf8" }
                    }
                    Text { anchors.centerIn: parent; text: "Apply"; color: "white"; font.pixelSize: 11; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (editorController) {
                                editorController.backgroundColor = bgColorField.text
                            }
                        }
                    }
                }
            }
            
            Text { text: "Cursor Color"; color: "#cbd5e1"; font.pixelSize: 12 }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                TextField {
                    id: cursorColorField
                    text: editorController ? editorController.cursorColor : "#ff0000"
                    onEditingFinished: if (editorController) editorController.cursorColor = text
                    Layout.fillWidth: true
                    color: "#f8fafc"
                    font.pixelSize: 12
                    background: Rectangle {
                        color: "#1e293b"
                        radius: 6
                        border.color: "#334155"
                    }
                }
                Rectangle {
                    width: 55
                    height: 32
                    radius: 6
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#0284c7" }
                        GradientStop { position: 1.0; color: "#38bdf8" }
                    }
                    Text { anchors.centerIn: parent; text: "Apply"; color: "white"; font.pixelSize: 11; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (editorController) editorController.cursorColor = cursorColorField.text
                    }
                }
            }
            
            Text { text: "Cursor Scale"; color: "#cbd5e1"; font.pixelSize: 12 }
            RowLayout {
                Layout.fillWidth: true
                SpinBox {
                    id: cursorScaleField
                    from: 10
                    to: 500
                    value: editorController ? Math.round(editorController.cursorScale * 100) : 100
                    onValueChanged: if (editorController) editorController.cursorScale = value / 100.0
                    Layout.fillWidth: true
                }
            }
            
            Item { Layout.fillHeight: true } // Spacer
            
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }
            
            Text { text: "🎨 STYLING PRESETS"; color: "#38bdf8"; font.pixelSize: 11; font.bold: true }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                ComboBox {
                    id: presetCombo
                    Layout.fillWidth: true
                    model: editorController ? editorController.getAvailablePresets() : []
                    onActivated: {
                        if (editorController) {
                            editorController.applyPreset(currentText);
                        }
                    }
                }
                Rectangle {
                    width: 34
                    height: 34
                    radius: 6
                    color: "#1e293b"
                    border.color: "#334155"
                    Text { anchors.centerIn: parent; text: "↻"; color: "#38bdf8"; font.pixelSize: 16 }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (editorController) {
                                presetCombo.model = editorController.getAvailablePresets()
                            }
                        }
                    }
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                TextField {
                    id: presetNameField
                    placeholderText: "Preset name..."
                    placeholderTextColor: "#64748b"
                    Layout.fillWidth: true
                    color: "#f8fafc"
                    font.pixelSize: 12
                    background: Rectangle {
                        color: "#1e293b"
                        radius: 6
                        border.color: "#334155"
                    }
                }
                Rectangle {
                    width: 50
                    height: 32
                    radius: 6
                    color: presetNameField.text.length > 0 ? "#10b981" : "#1e293b"
                    Text { anchors.centerIn: parent; text: "Save"; color: "white"; font.pixelSize: 11; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (editorController && presetNameField.text.length > 0) {
                                editorController.saveAsPreset(presetNameField.text);
                                presetNameField.text = "";
                                presetCombo.model = editorController.getAvailablePresets();
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 38
                radius: 8
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#059669" }
                    GradientStop { position: 1.0; color: "#10b981" }
                }
                Text { anchors.centerIn: parent; text: "💾 Save Project"; color: "white"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (editorController) editorController.saveProject()
                }
            }
        }
    }
    
    // Preview Area
    Rectangle {
        id: previewArea
        anchors.top: topBar.bottom
        anchors.bottom: timelineArea.top
        anchors.left: parent.left
        anchors.right: propertiesPanel.left
        color: (editorController && editorController.backgroundColor.startsWith("#")) ? editorController.backgroundColor : "#0a0a0a"
        clip: true
        
        // This is the target cropped canvas size (the aspect ratio container)
        property real targetW: editorController ? editorController.videoWidth : 1920
        property real targetH: editorController ? editorController.videoHeight : 1080
        property real scaleFactor: Math.min(previewArea.width / targetW, previewArea.height / targetH)
        property real videoPaddingScale: 0.88 // 12% padding around video container to display wallpaper background
        
        // The bounding box for the "Canvas" that the user is exporting
        Rectangle {
            id: exportCanvas
            width: previewArea.targetW * previewArea.scaleFactor
            height: previewArea.targetH * previewArea.scaleFactor
            anchors.centerIn: parent
            color: (editorController && editorController.backgroundColor.startsWith("#")) ? editorController.backgroundColor : "#0f172a"
            clip: true // This strictly clips everything outside the aspect ratio!
            
            // Recordly Wallpaper Background Image Layer
            Image {
                id: wallpaperBg
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                smooth: true
                mipmap: true
                visible: source.toString().length > 0
                source: {
                    var bg = editorController ? editorController.backgroundColor : ""
                    if (bg === "" || bg.startsWith("#")) return ""
                    if (bg.startsWith("file://") || bg.startsWith("qrc:/") || bg.startsWith("/")) return bg
                    var base = (typeof appDataUrl !== "undefined" ? appDataUrl : "")
                    return base + "/wallpapers/" + bg
                }
            }
            
            // Inside the canvas, we put the camera container (which is framed with videoPaddingScale)
            Item {
                id: cameraContainer
                width: previewArea.targetW
                height: previewArea.targetH
                
                property real camZoom: editorController ? editorController.cameraZoom : 1.0
                property real camX: editorController ? editorController.cameraX : width / 2
                property real camY: editorController ? editorController.cameraY : height / 2
                property real camRot: editorController ? editorController.cameraRotation : 0
                
                // Scale from top left
                transformOrigin: Item.TopLeft
                scale: previewArea.scaleFactor * previewArea.videoPaddingScale * camZoom
                rotation: camRot
                
                // Position it such that (camX, camY) of the raw video is exactly at the center of the exportCanvas
                x: (exportCanvas.width / 2) - (camX * scale)
                y: (exportCanvas.height / 2) - (camY * scale)

                MediaPlayer {
                    id: videoPlayer
                    source: editorController ? editorController.videoUrl : ""
                    videoOutput: videoOutput
                    audioOutput: null // We use separate audio files if available
                    
                    onPositionChanged: {
                        if (editorController && editorController.isPlaying) {
                            editorController.syncTime(videoPlayer.position);
                        }
                    }
                    
                    onDurationChanged: {
                        if (editorController && videoPlayer.duration > 0) {
                            editorController.updateDurationFromMedia(videoPlayer.duration);
                        }
                    }
                    
                    onMetaDataChanged: {
                        if (editorController && videoPlayer.hasVideo) {
                            var res = videoPlayer.metaData.videoResolution;
                            if (res && res.width > 0 && res.height > 0) {
                                editorController.updateResolutionFromMedia(res.width, res.height);
                            }
                        }
                    }
                    
                    onMediaStatusChanged: {
                        if (mediaStatus === MediaPlayer.LoadedMedia && editorController) {
                            if (videoPlayer.duration > 0) {
                                editorController.updateDurationFromMedia(videoPlayer.duration);
                            }
                            if (!editorController.isPlaying) {
                                videoPlayer.position = editorController.currentTime;
                                // Workaround for Qt6: play then pause instantly to force the first frame to render
                                videoPlayer.play();
                                videoPlayer.pause();
                            }
                        }
                    }
                }
                
                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                    fillMode: VideoOutput.Stretch // Stretching raw into raw is just 1:1 fitting
                    
                    Component.onCompleted: {
                        if (editorController) {
                            editorController.videoSink = videoOutput.videoSink;
                        }
                    }
                }

                MediaPlayer {
                    id: webcamPlayer
                    source: (editorController && editorController.webcamUrl) ? editorController.webcamUrl : ""
                    videoOutput: webcamOutput
                    audioOutput: null
                    
                    onMediaStatusChanged: {
                        if (mediaStatus === MediaPlayer.LoadedMedia && editorController && !editorController.isPlaying) {
                            webcamPlayer.position = editorController.currentTime;
                            // Workaround for Qt6: play then pause instantly to force the first frame to render
                            webcamPlayer.play();
                            webcamPlayer.pause();
                        }
                    }
                }
                
                Rectangle {
                    id: webcamMask
                    width: Math.min(parent.width, parent.height) * 0.25
                    height: width
                    radius: width / 2
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 40
                    color: "transparent"
                    border.color: "white"
                    border.width: 3
                    visible: webcamPlayer.hasVideo
                    clip: true
                    
                    VideoOutput {
                        id: webcamOutput
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectCrop
                    }
                }

                AudioOutput {
                    id: systemAudioOut
                }

                MediaPlayer {
                    id: systemAudioPlayer
                    source: (editorController && editorController.systemAudioUrl) ? editorController.systemAudioUrl : ""
                    audioOutput: systemAudioOut
                    
                    onMediaStatusChanged: {
                        if (mediaStatus === MediaPlayer.LoadedMedia && editorController && !editorController.isPlaying) {
                            systemAudioPlayer.position = editorController.currentTime;
                        }
                    }
                }
                
                AudioOutput {
                    id: micAudioOut
                }

                MediaPlayer {
                    id: micAudioPlayer
                    source: (editorController && editorController.micAudioUrl) ? editorController.micAudioUrl : ""
                    audioOutput: micAudioOut
                    
                    onMediaStatusChanged: {
                        if (mediaStatus === MediaPlayer.LoadedMedia && editorController && !editorController.isPlaying) {
                            micAudioPlayer.position = editorController.currentTime;
                        }
                    }
                }

                // Virtual Mouse Cursor Overlay
                Item {
                    id: virtualCursor
                    z: 100
                    visible: editorController ? (editorController.mouseEventCount > 0) : true
                    
                    property real baseSize: 28
                    property real curScale: editorController ? editorController.cursorScale : 1.0
                    property real bounceScale: editorController ? editorController.cursorBounceScale : 1.0
                    property real swayAngle: editorController ? editorController.cursorSwayAngle : 0.0
                    property real zoomScale: editorController ? editorController.cursorZoomScale : 1.0
                    property real anchorX: editorController ? editorController.cursorAnchorX : 0.34
                    property real anchorY: editorController ? editorController.cursorAnchorY : 0.24
                    property string curType: (editorController && editorController.cursorType !== "") ? editorController.cursorType : "arrow"
                    property real clickElapsed: editorController ? editorController.timeSinceClick : 9999
                    
                    x: editorController ? editorController.cursorX : 0
                    y: editorController ? editorController.cursorY : 0
                    rotation: swayAngle
                    
                    // 1. Primary Vector Click Ripple (Recordly Style 1)
                    Rectangle {
                        id: clickRipple
                        anchors.centerIn: parent
                        
                        property real tMs: parent.clickElapsed
                        property bool isActive: tMs >= 0 && tMs < 450
                        
                        visible: isActive
                        
                        property real clickProg: isActive ? (1.0 - tMs / 450.0) : 0 // 1.0 down to 0.0
                        property real eased: 1.0 - Math.pow(clickProg, 3.0) // 0.0 expanding out to 1.0
                        property real fade: Math.pow(clickProg, 3.0)
                        property real scaledH: (editorController ? editorController.cursorHeight : 40) * parent.curScale * parent.zoomScale
                        
                        width: Math.max(10, Math.round(eased * scaledH * 2.2))
                        height: width
                        radius: width / 2
                        
                        color: "transparent"
                        border.color: (editorController && editorController.cursorColor !== "") ? editorController.cursorColor : "#ff3366"
                        border.width: Math.max(2, Math.round(2.0 * (1.0 + fade)))
                        opacity: Math.max(0.0, Math.min(1.0, fade * 0.85))
                    }

                    // 2. Spotlight Concentric Inner Ring (Recordly Style 2)
                    Rectangle {
                        id: clickSpotlight
                        anchors.centerIn: parent
                        
                        property real tMs: parent.clickElapsed
                        property bool isActive: tMs >= 0 && tMs < 450
                        
                        visible: isActive
                        
                        property real clickProg: isActive ? (1.0 - tMs / 450.0) : 0
                        property real fade: Math.pow(clickProg, 2.0)
                        
                        width: Math.max(8, Math.round(clickRipple.width * 0.62))
                        height: width
                        radius: width / 2
                        
                        color: "transparent"
                        border.color: (editorController && editorController.cursorColor !== "") ? editorController.cursorColor : "#ff3366"
                        border.width: 1.5
                        opacity: Math.max(0.0, Math.min(0.6, fade * 0.5))
                    }
                    
                    // 3. Central Click Accent Dot (Recordly Echo Center Fill)
                    Rectangle {
                        id: clickCenterDot
                        anchors.centerIn: parent
                        
                        property real tMs: parent.clickElapsed
                        property bool isActive: tMs >= 0 && tMs < 300
                        
                        visible: isActive
                        
                        property real clickProg: isActive ? (1.0 - tMs / 300.0) : 0
                        property real scaledH: (editorController ? editorController.cursorHeight : 40) * parent.curScale * parent.zoomScale
                        
                        width: Math.max(4, Math.round(scaledH * 0.35 * clickProg))
                        height: width
                        radius: width / 2
                        
                        color: (editorController && editorController.cursorColor !== "") ? editorController.cursorColor : "#ff3366"
                        opacity: Math.max(0.0, Math.min(0.9, clickProg))
                    }
                    
                    // The Cursor Image Container (Hardware GPU Matrix Scaled around Hotspot)
                    Item {
                        id: cursorScaleContainer
                        x: 0
                        y: 0
                        scale: parent.curScale * parent.bounceScale * parent.zoomScale
                        transformOrigin: Item.TopLeft
                        
                        Image {
                            id: cursorImg
                            source: root.getCursorSource(virtualCursor.curType)
                            
                            property real cursorLogicalWidth: editorController ? editorController.cursorWidth : 28
                            property real cursorLogicalHeight: editorController ? editorController.cursorHeight : 40
                            
                            width: cursorLogicalWidth
                            height: cursorLogicalHeight
                            
                            x: -virtualCursor.anchorX * width
                            y: -virtualCursor.anchorY * height
                            
                            smooth: true
                            antialiasing: true
                            
                            onStatusChanged: {
                                if (status === Image.Error) {
                                    console.warn("Cursor image error for " + source + ". Falling back to PNG.")
                                    var base = (typeof appDataUrl !== "undefined" ? appDataUrl : "")
                                    var fallbackType = (virtualCursor.curType && virtualCursor.curType !== "") ? virtualCursor.curType : "arrow"
                                    var pngUrl = base + "/cursors/" + fallbackType + ".png"
                                    if (source.toString() !== pngUrl && source.toString() !== base + "/cursors/arrow.png") {
                                        source = pngUrl
                                    } else {
                                        source = base + "/cursors/arrow.png"
                                    }
                                }
                            }
                        }
                    }
                }
                
                // Subtitles Overlay
                Rectangle {
                    id: subtitleContainer
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 40
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "#A0000000" // Semi-transparent black
                    radius: 8
                    width: subtitleText.width + 32
                    height: subtitleText.height + 16
                    visible: editorController && editorController.subtitleText !== ""
                    
                    Text {
                        id: subtitleText
                        anchors.centerIn: parent
                        text: editorController ? editorController.subtitleText : ""
                        color: "white"
                        font.pixelSize: 24
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
    
    // Timeline Area (Frosted Glass Container)
    Rectangle {
        id: timelineArea
        width: parent.width
        height: 185
        anchors.bottom: parent.bottom
        color: "#b30f172a" // Translucent dark glass
        border.color: "#38bdf8"
        border.width: 1
        z: 10

        // Specular Rim Light
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: "#60ffffff"
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                // Play / Pause Glass Pill Button
                Rectangle {
                    width: 90
                    height: 32
                    radius: 8
                    gradient: Gradient {
                        GradientStop { 
                            position: 0.0
                            color: (editorController && editorController.isPlaying) ? "#d97706" : "#0284c7"
                        }
                        GradientStop { 
                            position: 1.0
                            color: (editorController && editorController.isPlaying) ? "#f59e0b" : "#38bdf8"
                        }
                    }
                    border.color: playMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: (editorController && editorController.isPlaying) ? "⏸ Pause" : "▶ Play"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: playMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (editorController) editorController.togglePlayback()
                        }
                    }
                }
                
                // Add Marker Button
                Rectangle {
                    width: 110
                    height: 32
                    radius: 8
                    color: "#1e293b"
                    border.color: markMouse.containsMouse ? "#38bdf8" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "📍 Add Marker"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: markMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (editorController) {
                                editorController.addMarker(editorController.currentTime, "Chapter", "#f39c12");
                            }
                        }
                    }
                }
                
                Text {
                    text: {
                        if (!editorController) return "0.0s / 0.0s"
                        let curSecs = (editorController.currentTime / 1000).toFixed(1)
                        let totSecs = (editorController.totalDuration / 1000).toFixed(1)
                        return curSecs + "s / " + totSecs + "s"
                    }
                    color: "#38bdf8"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
            
            // Custom Timeline Assembly
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                // Trimming Ruler
                TimelineRuler {
                    id: ruler
                    width: parent.width
                    anchors.top: parent.top
                    totalDuration: Math.max(1, editorController ? editorController.totalDuration : 100)
                    
                    // We initialize them to avoid zero-state bugs, but rely on Connections to keep them synced
                    trimStart: 0
                    trimEnd: totalDuration
                    
                    onTrimStartChanged: {
                        if (editorController && Math.abs(editorController.trimStart - trimStart) > 10) {
                            editorController.trimStart = trimStart;
                        }
                    }
                    onTrimEndChanged: {
                        if (editorController && Math.abs(editorController.trimEnd - trimEnd) > 10) {
                            editorController.trimEnd = trimEnd;
                        }
                    }
                    
                    Connections {
                        target: editorController
                        
                        function onTrimStartChanged() {
                            if (Math.abs(ruler.trimStart - editorController.trimStart) > 10) {
                                ruler.trimStart = editorController.trimStart;
                            }
                        }
                        
                        function onTrimEndChanged() {
                            // If it's a valid end time (> 0), sync it
                            if (editorController.trimEnd > 0 && Math.abs(ruler.trimEnd - editorController.trimEnd) > 10) {
                                ruler.trimEnd = editorController.trimEnd;
                            } else if (editorController.trimEnd === 0 && ruler.totalDuration > 0) {
                                // Default to total duration if uninitialized
                                ruler.trimEnd = ruler.totalDuration;
                            }
                        }
                        
                        function onProjectLoaded() {
                            onTrimStartChanged();
                            onTrimEndChanged();
                        }
                    }
                }
                
                // Track Container
                Column {
                    id: trackContainer
                    anchors.top: ruler.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    spacing: 4
                    
                    TimelineTrack {
                        width: parent.width
                        title: "Clip"
                        trackColor: "#d97706" // Amber/Orange
                        waveformData: editorController ? editorController.audioWaveform : []
                        isExtractingWaveform: editorController ? editorController.isExtractingWaveform : false
                    }
                    
                    TimelineTrack {
                        width: parent.width
                        title: "Zoom" // We could draw zoom keyframes here later
                        trackColor: "#7c3aed" // Purple
                    }
                }
                
                // Dimmed area for left trimmed region (covers tracks)
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.left
                    anchors.rightMargin: parent.width - ((ruler.trimStart / ruler.totalDuration) * parent.width)
                    anchors.top: trackContainer.top
                    anchors.bottom: parent.bottom
                    color: "#A0000000" // Dark overlay
                }
                
                // Dimmed area for right trimmed region (covers tracks)
                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: (ruler.trimEnd / ruler.totalDuration) * parent.width
                    anchors.right: parent.right
                    anchors.top: trackContainer.top
                    anchors.bottom: parent.bottom
                    color: "#A0000000" // Dark overlay
                }
                
                // Global Playhead
                Playhead {
                    anchors.top: ruler.bottom
                    anchors.bottom: parent.bottom
                    totalDuration: ruler.totalDuration
                    currentTime: editorController ? editorController.currentTime : 0
                    
                    onSeekRequested: (timeMs) => {
                        if (editorController) editorController.seek(timeMs)
                    }
                }
            }
            
            Connections {
                target: editorController
                
                function onDurationLoaded(duration) {
                    if (editorController && editorController.trimEnd === 0) {
                        editorController.trimEnd = duration;
                    }
                }
                
                function onPlaybackStateChanged(isPlaying) {
                    if (!editorController) return;
                    if (editorController.isPlaying) {
                        // Ensure perfect sync right before hitting play
                        videoPlayer.position = editorController.currentTime
                        if (webcamPlayer.hasVideo) webcamPlayer.position = editorController.currentTime
                        systemAudioPlayer.position = editorController.currentTime
                        micAudioPlayer.position = editorController.currentTime
                        
                        videoPlayer.play()
                        webcamPlayer.play()
                        systemAudioPlayer.play()
                        micAudioPlayer.play()
                    } else {
                        videoPlayer.pause()
                        webcamPlayer.pause()
                        systemAudioPlayer.pause()
                        micAudioPlayer.pause()
                        // Resync position on pause to ensure they are locked
                        videoPlayer.position = editorController.currentTime
                        if (webcamPlayer.hasVideo) webcamPlayer.position = editorController.currentTime
                        systemAudioPlayer.position = editorController.currentTime
                        micAudioPlayer.position = editorController.currentTime
                    }
                }
                
                function onCurrentTimeChanged() {
                    if (!editorController) return;
                    
                    // Enforce trim loop during playback
                    if (editorController.isPlaying && editorController.trimEnd > 0 && editorController.currentTime >= editorController.trimEnd) {
                        editorController.seek(editorController.trimStart);
                    }
                    
                    if (!editorController.isPlaying) {
                        // Always force seek when paused to guarantee visual updates while scrubbing
                        videoPlayer.position = editorController.currentTime;
                        if (webcamPlayer.hasVideo) webcamPlayer.position = editorController.currentTime;
                        systemAudioPlayer.position = editorController.currentTime;
                        micAudioPlayer.position = editorController.currentTime;
                    } else if (Math.abs(videoPlayer.position - editorController.currentTime) > 100) {
                        // When playing, only force seek if significantly out of sync
                        videoPlayer.position = editorController.currentTime;
                        webcamPlayer.position = editorController.currentTime;
                        systemAudioPlayer.position = editorController.currentTime;
                        micAudioPlayer.position = editorController.currentTime;
                    }
                }
            }
        }
    }
    
    // Add Marker Shortcut
    Shortcut {
        sequence: "B"
        onActivated: {
            if (editorController) {
                editorController.addMarker(editorController.currentTime, "Chapter", "#f39c12");
            }
        }
    }
}
