import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    
    // Properties to be set when opening this view
    property string screenshotPath: ""
    
    // Drawing states
    property var currentPath: []
    property var paths: []
    property color drawColor: "#ff0000"
    property int drawLineWidth: 4
    
    Rectangle {
        anchors.fill: parent
        color: "#070b19"
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            
            // Glassmorphic Toolbar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 54
                color: "#d90f172a"
                border.color: "#38bdf8"
                border.width: 1
                z: 10
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 14
                    
                    // Back Pill Button
                    Rectangle {
                        width: 90
                        height: 34
                        radius: 8
                        color: "#1e293b"
                        border.color: backMouse.containsMouse ? "#38bdf8" : "#334155"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "◀ Back"
                            color: "#f8fafc"
                            font.pixelSize: 12
                            font.bold: true
                        }

                        MouseArea {
                            id: backMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: screenshotEditor.visible = false
                        }
                    }
                    
                    Item { Layout.fillWidth: true } // Spacer
                    
                    // Color Palette Swatches
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 28; height: 28; radius: 14; color: "#ef4444"
                            border.color: root.drawColor === "#ff0000" || root.drawColor === "#ef4444" ? "#ffffff" : "#334155"
                            border.width: 2
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.drawColor = "#ef4444" }
                        }
                        Rectangle {
                            width: 28; height: 28; radius: 14; color: "#10b981"
                            border.color: root.drawColor === "#00ff00" || root.drawColor === "#10b981" ? "#ffffff" : "#334155"
                            border.width: 2
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.drawColor = "#10b981" }
                        }
                        Rectangle {
                            width: 28; height: 28; radius: 14; color: "#38bdf8"
                            border.color: root.drawColor === "#0000ff" || root.drawColor === "#38bdf8" ? "#ffffff" : "#334155"
                            border.width: 2
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.drawColor = "#38bdf8" }
                        }
                    }
                    
                    // Clear Pill Button
                    Rectangle {
                        width: 80
                        height: 34
                        radius: 8
                        color: "#1e293b"
                        border.color: clearMouse.containsMouse ? "#ef4444" : "#334155"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🗑 Clear"
                            color: "#f8fafc"
                            font.pixelSize: 12
                            font.bold: true
                        }

                        MouseArea {
                            id: clearMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.paths = []
                                drawingCanvas.requestPaint()
                            }
                        }
                    }
                    
                    Item { Layout.fillWidth: true } // Spacer
                    
                    // Save & Copy Primary Button
                    Rectangle {
                        id: saveCopyBtn
                        width: 135
                        height: 34
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#0284c7" }
                            GradientStop { position: 1.0; color: "#38bdf8" }
                        }
                        border.color: saveMouse.containsMouse ? "#ffffff" : "transparent"
                        border.width: 1.5

                        property string btnText: "📋 Save & Copy"

                        Text {
                            anchors.centerIn: parent
                            text: saveCopyBtn.btnText
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.bold: true
                        }

                        MouseArea {
                            id: saveMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                screenshotImage.grabToImage(function(result) {
                                    result.saveToFile(root.screenshotPath)
                                    clipboardManager.copyImageToClipboard(root.screenshotPath)
                                    saveCopyBtn.btnText = "✓ Saved & Copied!"
                                    copyTimer.start()
                                });
                            }
                        }

                        Timer {
                            id: copyTimer
                            interval: 2000
                            onTriggered: saveCopyBtn.btnText = "📋 Save & Copy"
                        }
                    }
                }
            }
            
            // Image and Canvas Container
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Image {
                    id: screenshotImage
                    anchors.centerIn: parent
                    width: parent.width * 0.9
                    height: parent.height * 0.9
                    source: root.screenshotPath ? "file://" + root.screenshotPath : ""
                    fillMode: Image.PreserveAspectFit
                    
                    // The drawing canvas overlays exactly on the image
                    Canvas {
                        id: drawingCanvas
                        anchors.fill: parent
                        
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);
                            
                            // Draw saved paths
                            for (var i = 0; i < root.paths.length; i++) {
                                var p = root.paths[i];
                                if (p.points.length < 2) continue;
                                
                                ctx.beginPath();
                                ctx.strokeStyle = p.color;
                                ctx.lineWidth = p.width;
                                ctx.lineCap = "round";
                                ctx.lineJoin = "round";
                                
                                ctx.moveTo(p.points[0].x, p.points[0].y);
                                for (var j = 1; j < p.points.length; j++) {
                                    ctx.lineTo(p.points[j].x, p.points[j].y);
                                }
                                ctx.stroke();
                            }
                            
                            // Draw current path
                            if (root.currentPath.length >= 2) {
                                ctx.beginPath();
                                ctx.strokeStyle = root.drawColor;
                                ctx.lineWidth = root.drawLineWidth;
                                ctx.lineCap = "round";
                                ctx.lineJoin = "round";
                                
                                ctx.moveTo(root.currentPath[0].x, root.currentPath[0].y);
                                for (var k = 1; k < root.currentPath.length; k++) {
                                    ctx.lineTo(root.currentPath[k].x, root.currentPath[k].y);
                                }
                                ctx.stroke();
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            
                            onPressed: (mouse) => {
                                root.currentPath = [{x: mouse.x, y: mouse.y}]
                            }
                            
                            onPositionChanged: (mouse) => {
                                if (root.currentPath.length > 0) {
                                    var newPath = root.currentPath
                                    newPath.push({x: mouse.x, y: mouse.y})
                                    root.currentPath = newPath
                                    drawingCanvas.requestPaint()
                                }
                            }
                            
                            onReleased: (mouse) => {
                                if (root.currentPath.length > 1) {
                                    var savedPaths = root.paths
                                    savedPaths.push({
                                        points: root.currentPath,
                                        color: root.drawColor,
                                        width: root.drawLineWidth
                                    })
                                    root.paths = savedPaths
                                }
                                root.currentPath = []
                                drawingCanvas.requestPaint()
                            }
                        }
                    }
                }
            }
        }
    }
}
