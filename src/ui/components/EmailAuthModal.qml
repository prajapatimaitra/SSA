import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: modal
    width: 440
    height: 380
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay

    property int currentStep: 1 // 1: Gmail & Password Input, 2: OTP Verification Code Input
    property string activeCode: ""

    function openModal() {
        currentStep = 1;
        emailInput.text = "";
        passwordInput.text = "";
        codeInput.text = "";
        activeCode = "";
        authManager.clearAuthError();
        open();
    }

    Connections {
        target: authManager
        function onVerificationCodeSent(email, code) {
            modal.activeCode = code;
            modal.currentStep = 2;
            codeInput.text = "";
        }
        function onLoginSuccess(name, email) {
            modal.close();
        }
    }

    background: Rectangle {
        color: "#0f172a"
        radius: 16
        border.color: "#0284c7"
        border.width: 1.5
    }

    contentItem: Item {
        anchors.fill: parent
        anchors.margins: 24

        // STEP 1: Enter Gmail & Password
        ColumnLayout {
            id: step1View
            anchors.fill: parent
            visible: modal.currentStep === 1
            spacing: 10

            RowLayout {
                spacing: 12
                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    color: "#0284c7"
                    Text {
                        anchors.centerIn: parent
                        text: "✉"
                        color: "#ffffff"
                        font.pixelSize: 20
                    }
                }
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "Sign in with Gmail"
                        color: "#f8fafc"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Text {
                        text: "Only @gmail.com accounts are permitted"
                        color: "#94a3b8"
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#1e293b"
            }

            Text {
                text: "Gmail Address:"
                color: "#cbd5e1"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                id: emailInput
                Layout.fillWidth: true
                placeholderText: "username@gmail.com"
                color: "#ffffff"
                font.pixelSize: 13
                selectByMouse: true
                background: Rectangle {
                    color: "#1e293b"
                    radius: 6
                    border.color: authManager.authError !== "" ? "#ef4444" : (emailInput.activeFocus ? "#38bdf8" : "#334155")
                    border.width: 1
                }
            }

            // User Password Field
            Text {
                text: "Account Password:"
                color: "#cbd5e1"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                id: passwordInput
                Layout.fillWidth: true
                placeholderText: "Enter account password"
                echoMode: TextInput.Password
                color: "#ffffff"
                font.pixelSize: 13
                selectByMouse: true
                background: Rectangle {
                    color: "#1e293b"
                    radius: 6
                    border.color: authManager.authError !== "" ? "#ef4444" : (passwordInput.activeFocus ? "#38bdf8" : "#334155")
                    border.width: 1
                }
                onAccepted: sendCodeBtn.clicked()
            }

            // Error Notice Banner
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: errorText.implicitHeight + 14
                radius: 6
                color: "#450a0a"
                border.color: "#ef4444"
                border.width: 1
                visible: authManager.authError !== ""

                Text {
                    id: errorText
                    anchors.fill: parent
                    anchors.margins: 6
                    text: authManager.authError
                    color: "#fca5a5"
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // Cancel Button
                Rectangle {
                    width: 90
                    height: 38
                    radius: 8
                    color: "#1e293b"
                    border.color: cancelAuthMouse.containsMouse ? "#38bdf8" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelAuthMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: modal.close()
                    }
                }

                Item { Layout.fillWidth: true }

                // Send Code Button
                Rectangle {
                    width: 185
                    height: 38
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#0284c7" }
                        GradientStop { position: 1.0; color: "#38bdf8" }
                    }
                    border.color: sendCodeMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "✉ Send Verification Code"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: sendCodeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var rawEmail = emailInput.text ? emailInput.text.trim() : "";
                            var rawPass = passwordInput.text ? passwordInput.text.trim() : "";
                            authManager.requestEmailVerificationWithPassword(rawEmail, rawPass);
                        }
                    }
                }
            }
        }

        // STEP 2: Enter Verification Code
        ColumnLayout {
            id: step2View
            anchors.fill: parent
            visible: modal.currentStep === 2
            spacing: 14

            RowLayout {
                spacing: 12
                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    color: "#16a34a"
                    Text {
                        anchors.centerIn: parent
                        text: "🔑"
                        font.pixelSize: 20
                    }
                }
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "Enter Verification Code"
                        color: "#f8fafc"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Text {
                        text: "Sent to " + authManager.pendingEmail
                        color: "#94a3b8"
                        font.pixelSize: 12
                    }
                }
            }

            // Code Notification Box
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 40
                radius: 6
                color: "#14532d"
                border.color: "#22c55e"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                    Text {
                        text: "✉ Code sent to " + authManager.pendingEmail + "! Please check your inbox."
                        color: "#86efac"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Text {
                text: "6-Digit Code:"
                color: "#cbd5e1"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                id: codeInput
                Layout.fillWidth: true
                placeholderText: "123456"
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: TextInput.AlignHCenter
                maximumLength: 6
                selectByMouse: true
                background: Rectangle {
                    color: "#1e293b"
                    radius: 8
                    border.color: authManager.authError !== "" ? "#ef4444" : (codeInput.activeFocus ? "#22c55e" : "#334155")
                    border.width: 1
                }
                onAccepted: {
                    if (authManager.verifyCode(codeInput.text)) {
                        modal.close();
                    }
                }
            }

            // Error Notice Banner for Code
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: codeErrorText.implicitHeight + 16
                radius: 8
                color: "#450a0a"
                border.color: "#ef4444"
                border.width: 1
                visible: authManager.authError !== ""

                Text {
                    id: codeErrorText
                    anchors.fill: parent
                    anchors.margins: 8
                    text: authManager.authError
                    color: "#fca5a5"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // Back Button
                Rectangle {
                    width: 90
                    height: 38
                    radius: 8
                    color: "#1e293b"
                    border.color: backStep2Mouse.containsMouse ? "#38bdf8" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "◀ Back"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: backStep2Mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            modal.currentStep = 1;
                            authManager.clearAuthError();
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Verify Button
                Rectangle {
                    width: 145
                    height: 38
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#15803d" }
                        GradientStop { position: 1.0; color: "#22c55e" }
                    }
                    border.color: verifyMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "✓ Verify & Sign In"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: verifyMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (authManager.verifyCode(codeInput.text)) {
                                modal.close();
                            }
                        }
                    }
                }
            }
        }
    }
}
