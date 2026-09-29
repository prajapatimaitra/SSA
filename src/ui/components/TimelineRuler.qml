import QtQuick
import QtQuick.Controls

Item {
    id: root
    height: 30

    property real totalDuration: 1 // prevent division by zero
    property real trimStart: 0
    property real trimEnd: totalDuration
    
    // Pixel to time conversion
    property real pixelsPerMs: width / totalDuration
    
    // Time ticks
    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            ctx.strokeStyle = "#555555";
            ctx.lineWidth = 1;
            ctx.fillStyle = "#888888";
            ctx.font = "10px sans-serif";
            
            // Draw a tick every 1 second
            var msPerTick = 1000;
            var numTicks = Math.ceil(totalDuration / msPerTick);
            
            for (var i = 0; i <= numTicks; i++) {
                var x = i * msPerTick * pixelsPerMs;
                
                // Major tick every 5 seconds
                var isMajor = (i % 5 === 0);
                var tickHeight = isMajor ? 12 : 6;
                
                ctx.beginPath();
                ctx.moveTo(x, height);
                ctx.lineTo(x, height - tickHeight);
                ctx.stroke();
                
                if (isMajor) {
                    var secs = i;
                    var mins = Math.floor(secs / 60);
                    secs = secs % 60;
                    var timeStr = mins + ":" + (secs < 10 ? "0" + secs : secs);
                    ctx.fillText(timeStr, x + 2, height - 14);
                }
            }
        }
    }
    
    // Request paint when dimensions or duration change
    onWidthChanged: canvas.requestPaint()
    onTotalDurationChanged: canvas.requestPaint()
    
    // Markers
    Repeater {
        model: editorController ? editorController.markers : []
        delegate: Item {
            // Using pixelsPerMs from the ruler
            x: (modelData.timestampMs / root.totalDuration) * root.width - width / 2
            y: root.height - 10
            width: 10
            height: 10
            
            // Draw a diamond/triangle shape for the marker
            Rectangle {
                anchors.fill: parent
                color: modelData.color
                rotation: 45
                border.color: "black"
                border.width: 1
            }
            
            ToolTip.visible: markerMouseArea.containsMouse
            ToolTip.text: modelData.note
            
            MouseArea {
                id: markerMouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton && editorController) {
                        editorController.removeMarker(index);
                    }
                }
                onDoubleClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton && editorController) {
                        renameMarkerPopup.markerIndex = index;
                        renameMarkerField.text = modelData.note;
                        renameMarkerPopup.open();
                        renameMarkerField.forceActiveFocus();
                    }
                }
            }
        }
    }
    
    // Rename Marker Popup
    Popup {
        id: renameMarkerPopup
        width: 200
        height: 60
        x: (parent.width - width) / 2
        y: -70
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        
        property int markerIndex: -1
        
        background: Rectangle {
            color: "#1e1e1e"
            radius: 4
            border.color: "#3a3a3a"
        }
        
        Row {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 5
            
            TextField {
                id: renameMarkerField
                width: 130
                placeholderText: "Marker name..."
                onAccepted: {
                    if (editorController && renameMarkerPopup.markerIndex >= 0) {
                        editorController.updateMarkerNote(renameMarkerPopup.markerIndex, text);
                    }
                    renameMarkerPopup.close();
                }
            }
            
            Button {
                text: "Save"
                width: 45
                onClicked: {
                    if (editorController && renameMarkerPopup.markerIndex >= 0) {
                        editorController.updateMarkerNote(renameMarkerPopup.markerIndex, renameMarkerField.text);
                    }
                    renameMarkerPopup.close();
                }
            }
        }
    }
    
    // Dimmed area for left trimmed region
    Rectangle {
        anchors.left: parent.left
        anchors.right: leftHandle.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#80000000" // 50% black
    }
    
    // Dimmed area for right trimmed region
    Rectangle {
        anchors.left: rightHandle.horizontalCenter
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#80000000" // 50% black
    }

    // Left Trim Handle
    Rectangle {
        id: leftHandle
        width: 12
        height: parent.height
        color: "#ffc107" // Amber/Yellow
        radius: 2
        
        Binding {
            target: leftHandle
            property: "x"
            value: (root.trimStart / root.totalDuration) * root.width - leftHandle.width / 2
            when: !leftDragArea.drag.active
            restoreMode: Binding.RestoreBinding
        }
        
        MouseArea {
            id: leftDragArea
            anchors.fill: parent
            drag.target: leftHandle
            drag.axis: Drag.XAxis
            drag.minimumX: -width / 2
            drag.maximumX: rightHandle.x - width
            
            onPositionChanged: {
                if (drag.active) {
                    var newTime = ((leftHandle.x + width / 2) / root.width) * root.totalDuration;
                    root.trimStart = Math.max(0, newTime);
                }
            }
        }
    }
    
    // Right Trim Handle
    Rectangle {
        id: rightHandle
        width: 12
        height: parent.height
        color: "#ffc107"
        radius: 2
        
        Binding {
            target: rightHandle
            property: "x"
            value: (root.trimEnd / root.totalDuration) * root.width - rightHandle.width / 2
            when: !rightDragArea.drag.active
            restoreMode: Binding.RestoreBinding
        }
        
        MouseArea {
            id: rightDragArea
            anchors.fill: parent
            drag.target: rightHandle
            drag.axis: Drag.XAxis
            drag.minimumX: leftHandle.x + width
            drag.maximumX: root.width - width / 2
            
            onPositionChanged: {
                if (drag.active) {
                    var newTime = ((rightHandle.x + width / 2) / root.width) * root.totalDuration;
                    root.trimEnd = Math.min(root.totalDuration, newTime);
                }
            }
        }
    }
    
    // Trim region dark overlays
    Rectangle {
        anchors.left: parent.left
        anchors.right: leftHandle.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#80000000" // 50% black
    }
    
    Rectangle {
        anchors.left: rightHandle.horizontalCenter
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#80000000"
    }
}
