import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root
    width: 600
    height: 400
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Dim the background when palette is open
    background: Rectangle {
        color: "transparent"
    }
    
    Overlay.modal: Rectangle {
        color: "#80000000" // 50% opacity black
    }
    
    // Main container (Frosted Glass)
    Rectangle {
        anchors.fill: parent
        color: "#e60f172a"
        radius: 16
        border.color: "#38bdf8"
        border.width: 1.5
        clip: true
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            
            // Search Input Header
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: "#1e293b"
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: "🔍"
                        font.pixelSize: 18
                    }

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: "Type a command or shortcut..."
                        placeholderTextColor: "#64748b"
                        font.pixelSize: 17
                        color: "#f8fafc"
                        
                        background: Rectangle {
                            color: "transparent"
                        }
                        
                        onTextChanged: {
                            updateFilter()
                        }
                        
                        Keys.onDownPressed: {
                            if (commandList.count > 0) {
                                commandList.currentIndex = (commandList.currentIndex + 1) % commandList.count
                            }
                        }
                        
                        Keys.onUpPressed: {
                            if (commandList.count > 0) {
                                commandList.currentIndex = (commandList.currentIndex - 1 + commandList.count) % commandList.count
                            }
                        }
                        
                        Keys.onReturnPressed: {
                            if (commandList.currentIndex >= 0 && commandList.currentIndex < commandList.count) {
                                executeCommand(commandList.currentIndex)
                            }
                        }
                    }
                }

                // Divider line
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#334155"
                }
            }
            
            // Command List
            ListView {
                id: commandList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: filteredCommandsModel
                
                delegate: Rectangle {
                    width: commandList.width
                    height: 52
                    color: commandList.currentIndex === index ? "#1e293b" : "transparent"
                    
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 4
                        color: "#38bdf8"
                        visible: commandList.currentIndex === index
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: commandList.currentIndex = index
                        onClicked: executeCommand(index)
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 14
                        
                        Text {
                            text: model.icon
                            color: "#38bdf8"
                            font.pixelSize: 18
                        }
                        
                        Text {
                            text: model.name
                            color: "#f8fafc"
                            font.pixelSize: 15
                            font.weight: commandList.currentIndex === index ? Font.Bold : Font.Normal
                            Layout.fillWidth: true
                        }
                        
                        Rectangle {
                            height: 22
                            width: shortcutText.implicitWidth + 16
                            radius: 6
                            color: "#090d16"
                            border.color: "#334155"
                            visible: model.shortcut !== ""

                            Text {
                                id: shortcutText
                                anchors.centerIn: parent
                                text: model.shortcut
                                color: "#94a3b8"
                                font.pixelSize: 11
                                font.bold: true
                            }
                        }
                    }
                }
            }
        }
    }
    
    ListModel {
        id: allCommandsModel
        
        ListElement {
            name: "Start Fullscreen Recording"
            action: "start_recording"
            icon: "⏺"
            shortcut: ""
        }
        ListElement {
            name: "Select Region to Record"
            action: "select_region"
            icon: "⛶"
            shortcut: ""
        }
        ListElement {
            name: "Take Screenshot"
            action: "take_screenshot"
            icon: "📸"
            shortcut: ""
        }
        ListElement {
            name: "Go to Workspace"
            action: "go_workspace"
            icon: "🏠"
            shortcut: ""
        }
        ListElement {
            name: "Export Project"
            action: "export_project"
            icon: "📤"
            shortcut: ""
        }
        ListElement {
            name: "Open Settings"
            action: "open_settings"
            icon: "⚙️"
            shortcut: ""
        }
    }
    
    ListModel {
        id: filteredCommandsModel
    }
    
    function updateFilter() {
        filteredCommandsModel.clear();
        var query = searchInput.text.toLowerCase();
        
        for (var i = 0; i < allCommandsModel.count; i++) {
            var item = allCommandsModel.get(i);
            if (query === "" || item.name.toLowerCase().indexOf(query) !== -1) {
                filteredCommandsModel.append({
                    name: item.name,
                    action: item.action,
                    icon: item.icon,
                    shortcut: item.shortcut
                });
            }
        }
        commandList.currentIndex = 0;
    }
    
    function executeCommand(index) {
        var action = filteredCommandsModel.get(index).action;
        root.close();
        
        switch (action) {
            case "start_recording":
                if (captureController) captureController.startCapture("Default", "display_0", "", 60, "original", 2, 0, 0, 0, 0);
                if (appState) appState.transitionTo(1); // RECORDING
                break;
            case "select_region":
                // Emit signal to let WorkspaceView handle region selector
                root.triggerRegionSelector();
                break;
            case "take_screenshot":
                // Mocking screenshot action for now
                console.log("Command: Take Screenshot");
                break;
            case "go_workspace":
                if (appState) appState.transitionTo(0); // WORKSPACE
                break;
            case "export_project":
                // This would be active if we were in the editor
                console.log("Command: Export Project");
                break;
            case "open_settings":
                console.log("Command: Open Settings");
                break;
        }
    }
    
    signal triggerRegionSelector()
    
    onOpened: {
        searchInput.text = "";
        updateFilter();
        searchInput.forceActiveFocus();
    }
}
