import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#18181b"
    border.color: "#27272a"
    border.width: 1

    signal binSelected(int binId)
    signal newBinRequested(int parentBinId)

    property int currentSelectedBinId: 1

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            color: "#202023"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    text: "PROJECT BINS"
                    color: "#a1a1aa"
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 0.5
                    Layout.fillWidth: true
                }

                Button {
                    text: "+ Bin"
                    font.pixelSize: 11
                    font.weight: Font.Medium
                    flat: true
                    background: Rectangle {
                        color: parent.hovered ? "#3f3f46" : "#27272a"
                        radius: 4
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#f4f4f5"
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        if (typeof projectBinManager !== "undefined") {
                            projectBinManager.createBin("New Bin", root.currentSelectedBinId, "#6366f1")
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#27272a"
        }

        // Bins List View
        ListView {
            id: binListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: typeof projectBinManager !== "undefined" ? projectBinManager.binModel : null

            delegate: Rectangle {
                id: binRow
                width: binListView.width
                height: 36
                color: root.currentSelectedBinId === model.binId ? "#27272a" : (mouseArea.containsMouse ? "#1f1f23" : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8 + (model.depth * 16)
                    anchors.rightMargin: 8
                    spacing: 8

                    // Color Indicator Badge
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: model.colorTag ? model.colorTag : "#6366f1"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Folder Icon
                    Text {
                        text: "📁"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Bin Name
                    Text {
                        text: model.name
                        color: root.currentSelectedBinId === model.binId ? "#ffffff" : "#d4d4d8"
                        font.pixelSize: 13
                        font.weight: root.currentSelectedBinId === model.binId ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: (mouse) => {
                        root.currentSelectedBinId = model.binId
                        root.binSelected(model.binId)
                        if (typeof projectBinManager !== "undefined") {
                            projectBinManager.selectedBinId = model.binId
                        }
                        if (mouse.button === Qt.RightButton) {
                            binContextMenu.binId = model.binId
                            binContextMenu.binName = model.name
                            binContextMenu.popup()
                        }
                    }
                }
            }
        }
    }

    // Context Menu for Bin operations
    Menu {
        id: binContextMenu
        property int binId: -1
        property string binName: ""

        MenuItem {
            text: "Create Sub-Bin"
            onTriggered: {
                if (typeof projectBinManager !== "undefined" && binContextMenu.binId > 0) {
                    projectBinManager.createBin("Sub Bin", binContextMenu.binId, "#8b5cf6")
                }
            }
        }
        MenuItem {
            text: "Rename Bin"
            onTriggered: {
                renameDialog.binId = binContextMenu.binId
                renameDialog.currentName = binContextMenu.binName
                renameDialog.open()
            }
        }
        MenuItem {
            text: "Delete Bin"
            enabled: binContextMenu.binId > 1 // Don't delete master
            onTriggered: {
                if (typeof projectBinManager !== "undefined") {
                    projectBinManager.deleteBin(binContextMenu.binId)
                }
            }
        }
    }

    // Rename Dialog
    Dialog {
        id: renameDialog
        title: "Rename Bin"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        anchors.centerIn: Overlay.overlay

        property int binId: -1
        property string currentName: ""

        onOpened: {
            nameInput.text = currentName
            nameInput.selectAll()
            nameInput.forceActiveFocus()
        }

        onAccepted: {
            if (typeof projectBinManager !== "undefined" && binId > 0) {
                projectBinManager.renameBin(binId, nameInput.text)
            }
        }

        ColumnLayout {
            spacing: 10
            Text {
                text: "Enter new bin name:"
                color: "#e4e4e7"
                font.pixelSize: 13
            }
            TextField {
                id: nameInput
                Layout.fillWidth: true
                placeholderText: "Bin Name"
            }
        }
    }
}
