import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    implicitWidth: badgeRow.implicitWidth + 24
    implicitHeight: 36

    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: 18
        color: mouseArea.containsMouse ? "#d92a374a" : "#b31e293b"
        border.color: authManager.accountTier === 2 ? "#eab308" : (authManager.isLoggedIn ? "#38bdf8" : "#40ffffff")
        border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }

        // Specular Rim Highlight
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            height: 1
            color: "#60ffffff"
        }

        RowLayout {
            id: badgeRow
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 12
            spacing: 8

            // Avatar / Icon Circle
            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 12
                color: authManager.accountTier === 2 ? "#eab308" : (authManager.isLoggedIn ? "#0284c7" : "#475569")

                Text {
                    anchors.centerIn: parent
                    text: authManager.isLoggedIn ? (authManager.userName.length > 0 ? authManager.userName.charAt(0).toUpperCase() : "U") : "G"
                    color: authManager.accountTier === 2 ? "#000000" : "#ffffff"
                    font.bold: true
                    font.pixelSize: 12
                }
            }

            // User Name or Google Sign In Text
            Text {
                text: authManager.isAuthenticating ? "Connecting..." : (authManager.isLoggedIn ? authManager.userName : "Sign in with Google")
                color: "#f8fafc"
                font.pixelSize: 13
                font.weight: Font.Medium
            }

            // Tier Tag Badge
            Rectangle {
                visible: authManager.isLoggedIn
                Layout.preferredWidth: tierText.implicitWidth + 10
                Layout.preferredHeight: 18
                radius: 9
                color: authManager.accountTier === 2 ? "#eab308" : "#0284c7"

                Text {
                    id: tierText
                    anchors.centerIn: parent
                    text: authManager.accountTierName.toUpperCase()
                    color: authManager.accountTier === 2 ? "#000000" : "#ffffff"
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            Text {
                text: "▾"
                color: "#94a3b8"
                font.pixelSize: 11
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                if (!authManager.isLoggedIn) {
                    if (typeof emailAuthModal !== "undefined") {
                        emailAuthModal.openModal();
                    } else {
                        authManager.loginWithGoogle();
                    }
                } else {
                    profileMenu.open();
                }
            }
        }
    }

    // Dropdown Menu for Logged In User
    Menu {
        id: profileMenu
        y: root.height + 6
        x: root.width - width
        width: 260
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#0f172a"
            border.color: "#334155"
            border.width: 1
            radius: 12
        }

        contentItem: ColumnLayout {
            spacing: 12
            anchors.margins: 16

            // Header User Info
            RowLayout {
                spacing: 10
                Rectangle {
                    width: 36
                    height: 36
                    radius: 18
                    color: authManager.accountTier === 2 ? "#eab308" : "#0284c7"
                    Text {
                        anchors.centerIn: parent
                        text: authManager.userName.length > 0 ? authManager.userName.charAt(0).toUpperCase() : "U"
                        color: authManager.accountTier === 2 ? "#000000" : "#ffffff"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: authManager.userName
                        color: "#f8fafc"
                        font.bold: true
                        font.pixelSize: 14
                    }
                    Text {
                        text: authManager.userEmail
                        color: "#94a3b8"
                        font.pixelSize: 11
                        elide: Text.ElideRight
                        Layout.maximumWidth: 170
                    }
                }
            }

            // Custom Username Setting
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: "Unique Display Username:"
                    color: "#64748b"
                    font.pixelSize: 10
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    TextField {
                        id: customUsernameField
                        text: authManager.userName
                        Layout.fillWidth: true
                        font.pixelSize: 12
                        placeholderText: "Choose username"
                        color: "#ffffff"
                        background: Rectangle {
                            color: "#1e293b"
                            radius: 6
                            border.color: "#334155"
                            border.width: 1
                        }
                    }

                    Rectangle {
                        width: 50
                        height: 30
                        radius: 6
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#0284c7" }
                            GradientStop { position: 1.0; color: "#38bdf8" }
                        }
                        Text { anchors.centerIn: parent; text: "Save"; color: "white"; font.pixelSize: 11; font.bold: true }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: authManager.setCustomUsername(customUsernameField.text)
                        }
                    }
                }
            }

            // Owner Admin Dashboard Button
            Rectangle {
                Layout.fillWidth: true
                height: 34
                radius: 8
                visible: authManager.isAdmin
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#7c3aed" }
                    GradientStop { position: 1.0; color: "#8b5cf6" }
                }
                border.color: adminBadgeMouse.containsMouse ? "#c084fc" : "transparent"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "📊 Owner Admin Dashboard"
                    color: "#ffffff"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: adminBadgeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        profileMenu.close();
                        if (typeof adminDashboardModal !== "undefined") {
                            adminDashboardModal.openDashboard();
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }

            // Quota Limits Summary
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: "Plan & Usage Limits"
                    color: "#64748b"
                    font.pixelSize: 11
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Account Plan:"; color: "#94a3b8"; font.pixelSize: 12 }
                    Item { Layout.fillWidth: true }
                    Text { text: authManager.accountTierName; color: authManager.accountTier === 2 ? "#eab308" : "#38bdf8"; font.bold: true; font.pixelSize: 12 }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Exports Today:"; color: "#94a3b8"; font.pixelSize: 12 }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: quotaManager.dailyExportLimit < 0 ? "Unlimited" : (quotaManager.dailyExportCount + " / " + quotaManager.dailyExportLimit + " used")
                        color: "#f8fafc"
                        font.pixelSize: 12
                    }
                }

                // Quota Usage Progress Bar
                Rectangle {
                    visible: quotaManager.dailyExportLimit > 0
                    Layout.fillWidth: true
                    height: 5
                    radius: 2.5
                    color: "#1e293b"

                    Rectangle {
                        width: parent.width * Math.min(1.0, (quotaManager.dailyExportCount / Math.max(1, quotaManager.dailyExportLimit)))
                        height: parent.height
                        radius: 2.5
                        color: (quotaManager.dailyExportCount >= quotaManager.dailyExportLimit) ? "#ef4444" : "#38bdf8"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Max Duration:"; color: "#94a3b8"; font.pixelSize: 12 }
                    Item { Layout.fillWidth: true }
                    Text { text: quotaManager.maxDurationFormatted; color: "#f8fafc"; font.pixelSize: 12 }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }

            // Tier Switcher (Quick Simulation Demo for Testing)
            Text {
                text: "Quick Tier Simulation (Demo):"
                color: "#64748b"
                font.pixelSize: 10
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Rectangle {
                    Layout.fillWidth: true
                    height: 26
                    radius: 6
                    color: "#1e293b"
                    border.color: gstMouse.containsMouse ? "#38bdf8" : "#334155"
                    Text { anchors.centerIn: parent; text: "Guest"; color: "#cbd5e1"; font.pixelSize: 10; font.bold: true }
                    MouseArea {
                        id: gstMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { authManager.setSimulatedTier(0); profileMenu.close(); }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 26
                    radius: 6
                    color: "#1e293b"
                    border.color: freeMouse.containsMouse ? "#38bdf8" : "#334155"
                    Text { anchors.centerIn: parent; text: "Free"; color: "#38bdf8"; font.pixelSize: 10; font.bold: true }
                    MouseArea {
                        id: freeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { authManager.setSimulatedTier(1); profileMenu.close(); }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 26
                    radius: 6
                    color: "#172554"
                    border.color: proMouse.containsMouse ? "#f59e0b" : "#1e40af"
                    Text { anchors.centerIn: parent; text: "Pro"; color: "#f59e0b"; font.pixelSize: 10; font.bold: true }
                    MouseArea {
                        id: proMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: { authManager.setSimulatedTier(2); profileMenu.close(); }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }

            // Logout Glass Button
            Rectangle {
                Layout.fillWidth: true
                height: 32
                radius: 6
                color: "#1e293b"
                border.color: logoutMouse.containsMouse ? "#ef4444" : "#334155"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: authManager.isLoggedIn ? "🚪 Sign Out" : "🔑 Sign In"
                    color: authManager.isLoggedIn ? "#ef4444" : "#38bdf8"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: logoutMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (authManager.isLoggedIn) {
                            authManager.logout();
                        } else {
                            emailAuthModal.openModal();
                        }
                        profileMenu.close();
                    }
                }
            }
        }
    }
}
