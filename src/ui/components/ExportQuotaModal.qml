import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: modal
    width: 460
    height: 400
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay

    signal openEmailAuth()

    property string titleText: "Export Limit Reached"
    property string messageText: "Your video exceeds the export limit for your current account plan."

    function showModal(title, message) {
        titleText = title;
        messageText = message;
        open();
    }

    background: Rectangle {
        color: "#0f172a"
        radius: 16
        border.color: "#38bdf8"
        border.width: 1.5
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Header Icon & Title
        RowLayout {
            spacing: 12
            Rectangle {
                width: 44
                height: 44
                radius: 22
                color: "#1e293b"
                border.color: "#eab308"
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "⚡"
                    font.pixelSize: 22
                }
            }

            ColumnLayout {
                spacing: 2
                Text {
                    text: modal.titleText
                    color: "#f8fafc"
                    font.pixelSize: 18
                    font.bold: true
                }
                Text {
                    text: "Account Tier: " + authManager.accountTierName
                    color: authManager.accountTier === 2 ? "#eab308" : "#38bdf8"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }
            }
        }

        // Message body
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            radius: 8
            color: "#1e293b"
            border.color: "#334155"

            Text {
                anchors.fill: parent
                anchors.margins: 10
                text: modal.messageText
                color: "#cbd5e1"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }
        }

        // Tier Breakdown Card
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: "Plan Comparison:"
                color: "#94a3b8"
                font.pixelSize: 11
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 90
                color: "#090d16"
                radius: 8
                border.color: "#1e293b"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    // Guest Tier Box
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 6
                        color: authManager.accountTier === 0 ? "#1e293b" : "transparent"
                        border.color: authManager.accountTier === 0 ? "#38bdf8" : "transparent"
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Text { text: "Guest"; color: "#94a3b8"; font.bold: true; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "2 Mins Max"; color: "#cbd5e1"; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "2 Exports/day"; color: "#cbd5e1"; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
                        }
                    }

                    // Free Google Tier Box
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 6
                        color: authManager.accountTier === 1 ? "#1e293b" : "transparent"
                        border.color: authManager.accountTier === 1 ? "#38bdf8" : "transparent"
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Text { text: "Google Free"; color: "#38bdf8"; font.bold: true; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "5 Mins Max"; color: "#cbd5e1"; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "5 Exports/day"; color: "#cbd5e1"; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
                        }
                    }

                    // Pro Tier Box
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 6
                        color: authManager.accountTier === 2 ? "#1e293b" : "#172554"
                        border.color: "#eab308"
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Text { text: "Pro Plan"; color: "#eab308"; font.bold: true; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "Unlimited"; color: "#ffffff"; font.pixelSize: 10; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                            Text { text: "4K 60FPS"; color: "#ffffff"; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Actions (Glass Pill Buttons)
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Cancel Glass Button
            Rectangle {
                width: 100
                height: 38
                radius: 8
                color: "#1e293b"
                border.color: cancelQuotaMouse.containsMouse ? "#38bdf8" : "#334155"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Cancel"
                    color: "#f8fafc"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: cancelQuotaMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: modal.close()
                }
            }

            Item { Layout.fillWidth: true }

            // CTA Glass Button
            Rectangle {
                width: 180
                height: 38
                radius: 8
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#0284c7" }
                    GradientStop { position: 1.0; color: "#38bdf8" }
                }
                border.color: ctaQuotaMouse.containsMouse ? "#ffffff" : "transparent"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: !authManager.isLoggedIn ? "Sign in with Google" : "Upgrade to Pro"
                    color: "#ffffff"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: ctaQuotaMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        modal.close();
                        if (!authManager.isLoggedIn) {
                            modal.openEmailAuth();
                        } else {
                            authManager.setSimulatedTier(2);
                        }
                    }
                }
            }
        }
    }
}
