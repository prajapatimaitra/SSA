import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#09090b"

    signal assetSelected(int assetId, string filePath, string mediaType)

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // Left Sidebar: Bins Tree View
        BinTreeView {
            SplitView.minimumWidth: 180
            SplitView.preferredWidth: 220
            SplitView.maximumWidth: 360

            onBinSelected: (binId) => {
                if (typeof projectBinManager !== "undefined") {
                    projectBinManager.selectedBinId = binId
                }
            }
        }

        // Right Main Panel: Asset Grid View
        AssetGridView {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300

            onAssetDoubleClicked: (assetId, filePath, mediaType) => {
                root.assetSelected(assetId, filePath, mediaType)
            }
        }
    }
}
