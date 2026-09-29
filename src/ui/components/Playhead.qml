import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 14
    
    property real currentTime: 0
    property real totalDuration: 1
    property real containerWidth: parent ? parent.width : 0
    
    Binding {
        target: root
        property: "x"
        value: (currentTime / totalDuration) * containerWidth - width / 2
        when: !mouseArea.drag.active
        restoreMode: Binding.RestoreBinding
    }
    
    // The vertical line
    Rectangle {
        width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#00d2ff" // Cyan/Blue accent
    }
    
    // The handle cap at the top
    Rectangle {
        width: 14
        height: 14
        radius: 7
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: "#00d2ff"
        border.color: "#ffffff"
        border.width: 2
    }
    
    // Make it draggable
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.margins: -10 // larger hit area
        drag.target: root
        drag.axis: Drag.XAxis
        drag.minimumX: -width / 2
        drag.maximumX: containerWidth - width / 2
        
        onPositionChanged: {
            if (drag.active) {
                var newTime = ((root.x + root.width / 2) / containerWidth) * totalDuration;
                root.seekRequested(Math.max(0, Math.min(totalDuration, newTime)));
            }
        }
    }
    
    signal seekRequested(real timeMs)
}
