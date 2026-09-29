import QtQuick
import QtQuick.Controls

Item {
    id: root
    height: 40
    
    property string title: "Track"
    property color trackColor: "#4f4f4f"
    property var waveformData: [] // Array of numbers from 0.0 to 1.0
    property bool isExtractingWaveform: false
    
    // Background track container
    Rectangle {
        anchors.fill: parent
        color: "#2a2a2a"
        radius: 4
        border.color: "#3a3a3a"
        border.width: 1
        clip: true
        
        // The actual track colored bar (spans full width, or could represent total duration)
        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            color: root.trackColor
            radius: 3
            opacity: 0.8
            
            // Waveform canvas
            Canvas {
                id: waveCanvas
                anchors.fill: parent
                visible: root.waveformData && root.waveformData.length > 0
                
                onPaint: {
                    if (!root.waveformData || root.waveformData.length === 0) return;
                    
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    
                    ctx.fillStyle = "#ffffff";
                    ctx.globalAlpha = 0.5;
                    
                    var data = root.waveformData;
                    var numPoints = data.length;
                    
                    // Bar width and spacing
                    var step = Math.max(1, width / numPoints);
                    var barWidth = Math.max(1, step - 0.5);
                    
                    for (var i = 0; i < numPoints; i++) {
                        var h = data[i] * height * 0.9; // 90% of track height max
                        h = Math.max(2, h); // Min height of 2px
                        
                        var x = i * step;
                        var y = (height - h) / 2;
                        
                        ctx.fillRect(x, y, barWidth, h);
                    }
                }
            }
            
            // Loading indicator for waveform
            Text {
                anchors.centerIn: parent
                text: "Generating Waveform..."
                color: "white"
                opacity: 0.5
                font.pixelSize: 12
                visible: root.isExtractingWaveform && (!root.waveformData || root.waveformData.length === 0)
            }
        }
    }
    
    // Track Label
    Rectangle {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 8
        color: "#1e1e1e"
        radius: 4
        width: labelText.width + 12
        height: labelText.height + 6
        border.color: "#3a3a3a"
        
        Text {
            id: labelText
            anchors.centerIn: parent
            text: root.title
            color: "#aaaaaa"
            font.pixelSize: 11
            font.bold: true
        }
    }
    
    // Listen to changes in waveform data to repaint
    onWaveformDataChanged: {
        if (waveCanvas) waveCanvas.requestPaint();
    }
    onWidthChanged: {
        if (waveCanvas) waveCanvas.requestPaint();
    }
}
