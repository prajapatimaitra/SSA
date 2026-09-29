import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: root
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    x: Screen.virtualX
    y: Screen.virtualY
    
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowFullscreenButtonHint
    color: "transparent"
    
    signal regionSelected(int rx, int ry, int rw, int rh)
    signal canceled()
    
    property int startX: 0
    property int startY: 0
    property int currentX: 0
    property int currentY: 0
    property bool isDrawing: false

    property int selX: Math.min(root.startX, root.currentX)
    property int selY: Math.min(root.startY, root.currentY)
    property int selW: Math.abs(root.currentX - root.startX)
    property int selH: Math.abs(root.currentY - root.startY)

    // Full screen dim when not drawing
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: !root.isDrawing
    }

    // Four dimming rectangles when drawing
    Rectangle { // Top
        x: 0; y: 0
        width: root.width
        height: root.isDrawing ? root.selY : 0
        color: "#80000000"
        visible: root.isDrawing
    }
    Rectangle { // Bottom
        x: 0; y: root.isDrawing ? (root.selY + root.selH) : 0
        width: root.width
        height: root.isDrawing ? (root.height - (root.selY + root.selH)) : 0
        color: "#80000000"
        visible: root.isDrawing
    }
    Rectangle { // Left
        x: 0; y: root.isDrawing ? root.selY : 0
        width: root.isDrawing ? root.selX : 0
        height: root.isDrawing ? root.selH : 0
        color: "#80000000"
        visible: root.isDrawing
    }
    Rectangle { // Right
        x: root.isDrawing ? (root.selX + root.selW) : 0
        y: root.isDrawing ? root.selY : 0
        width: root.isDrawing ? (root.width - (root.selX + root.selW)) : 0
        height: root.isDrawing ? root.selH : 0
        color: "#80000000"
        visible: root.isDrawing
    }

    // The clear cut-out rectangle with border
    Rectangle {
        x: root.selX
        y: root.selY
        width: root.selW
        height: root.selH
        color: "transparent"
        border.color: "#3498db"
        border.width: 2
        visible: root.isDrawing

        // Dimension Label
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 4
            width: dimText.width + 12
            height: dimText.height + 6
            color: "#80000000"
            radius: 4
            
            Text {
                id: dimText
                anchors.centerIn: parent
                text: Math.round(root.selW) + " x " + Math.round(root.selH)
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }
        }
    }
    
    // Help text
    Text {
        anchors.centerIn: parent
        text: "Click and drag to select recording region.\nPress ESC to cancel."
        color: "white"
        font.pixelSize: 24
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        style: Text.Outline
        styleColor: "black"
        visible: !root.isDrawing
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.CrossCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        onPressed: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                root.canceled();
                root.hide();
                return;
            }
            root.startX = mouse.x;
            root.startY = mouse.y;
            root.currentX = mouse.x;
            root.currentY = mouse.y;
            root.isDrawing = true;
        }
        
        onPositionChanged: (mouse) => {
            if (root.isDrawing) {
                root.currentX = mouse.x;
                root.currentY = mouse.y;
            }
        }
        
        onReleased: (mouse) => {
            if (mouse.button === Qt.RightButton) return;
            root.isDrawing = false;
            var w = root.selW;
            var h = root.selH;
            if (w > 10 && h > 10) {
                root.regionSelected(root.selX, root.selY, w, h);
            } else {
                root.canceled();
            }
            root.hide();
        }
    }
    
    Item {
        focus: true
        Keys.onEscapePressed: {
            root.canceled();
            root.hide();
        }
    }
}
