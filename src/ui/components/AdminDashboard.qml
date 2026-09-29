import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: dashboardModal
    width: 900
    height: 700
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay

    property int activeTab: 0 // 0: Tenants Directory, 1: Solo Users Directory
    property var soloUsersList: []
    property var tenantsList: []
    property var selectedTenantUsersList: []
    property string activeTenantName: ""
    property string searchQuery: ""

    function openDashboard() {
        if (typeof userManager !== "undefined") {
            userManager.refreshStats();
            refreshData();
        }
        open();
    }

    function refreshData() {
        if (typeof userManager !== "undefined") {
            soloUsersList = userManager.getSoloUsersList();
            tenantsList = userManager.getTenantsList();
        }
    }

    function viewTenantUsers(tenantId) {
        if (typeof userManager !== "undefined") {
            activeTenantName = tenantId;
            selectedTenantUsersList = userManager.getTenantUsers(tenantId);
            tenantUsersModal.open();
        }
    }

    function configureTenantLimits(tenantId) {
        if (typeof userManager !== "undefined") {
            activeTenantName = tenantId;
            var limits = userManager.getTenantLimits(tenantId);
            tenantLimitDailyInput.text = limits.dailyLimit;
            tenantLimitDurationInput.text = limits.maxDurationMins;
            
            var res = limits.maxResolutionHeight;
            tenantLimitResCombo.currentIndex = (res >= 2160) ? 2 : ((res >= 1440) ? 1 : 0);
            tenantLimitsModal.open();
        }
    }

    function applyPresetToTenant(daily, durationMins, resIndex) {
        tenantLimitDailyInput.text = daily;
        tenantLimitDurationInput.text = durationMins;
        tenantLimitResCombo.currentIndex = resIndex;
    }

    function configureSoloPolicy() {
        if (typeof userManager !== "undefined") {
            var policy = userManager.getSoloPolicy();
            soloLimitDailyInput.text = policy.dailyLimit;
            soloLimitDurationInput.text = policy.maxDurationMins;
            
            var res = policy.maxResolutionHeight;
            soloLimitResCombo.currentIndex = (res >= 2160) ? 2 : ((res >= 1440) ? 1 : 0);
            soloLimitsModal.open();
        }
    }

    Connections {
        target: typeof userManager !== "undefined" ? userManager : null
        function onUsersChanged() {
            refreshData();
            if (tenantUsersModal.opened && activeTenantName !== "") {
                selectedTenantUsersList = userManager.getTenantUsers(activeTenantName);
            }
        }
    }

    background: Rectangle {
        radius: 16
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#090d16" }
            GradientStop { position: 1.0; color: "#0f172a" }
        }
        border.color: "#38bdf8"
        border.width: 1.5

        // Glowing Ambient Backdrop Shadow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            radius: 17
            color: "transparent"
            border.color: "#0284c7"
            border.width: 1
            opacity: 0.4
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Header Section
        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Rectangle {
                width: 48
                height: 48
                radius: 14
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#0284c7" }
                    GradientStop { position: 1.0; color: "#0369a1" }
                }
                Text {
                    anchors.centerIn: parent
                    text: "⚡"
                    color: "#ffffff"
                    font.pixelSize: 24
                }
            }

            ColumnLayout {
                spacing: 2
                Text {
                    text: "Admin Master Control Panel"
                    color: "#f8fafc"
                    font.pixelSize: 22
                    font.bold: true
                }
                Text {
                    text: "Real-time Multi-Tenant Management, Custom Export Limits & Mail Gateways"
                    color: "#94a3b8"
                    font.pixelSize: 12
                }
            }

            Item { Layout.fillWidth: true }

            // Close Pill Button
            Rectangle {
                width: 90
                height: 34
                radius: 8
                color: "#1e293b"
                border.color: closeAdminMouse.containsMouse ? "#ef4444" : "#334155"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "✕ Close"
                    color: "#f8fafc"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: closeAdminMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: dashboardModal.close()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#1e293b"
        }

        // Mail Gateway Bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 64
            radius: 10
            color: "#0d1527"
            border.color: "#1d2e4a"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    text: "⚙ Verification Gateway:"
                    color: "#38bdf8"
                    font.pixelSize: 11
                    font.bold: true
                }

                TextField {
                    id: adminSenderEmailField
                    Layout.fillWidth: true
                    placeholderText: "Sender Email (e.g. onboarding@resend.dev)"
                    text: authManager.senderEmail
                    color: "#ffffff"
                    font.pixelSize: 11
                    background: Rectangle { color: "#162032"; radius: 6; border.color: "#273852" }
                }

                TextField {
                    id: adminApiKeyField
                    Layout.fillWidth: true
                    placeholderText: "API Key (re_xxx / xkeysib-xxx)"
                    text: authManager.mailApiKey
                    echoMode: TextInput.Password
                    color: "#ffffff"
                    font.pixelSize: 11
                    background: Rectangle { color: "#162032"; radius: 6; border.color: "#273852" }
                }

                // Save Config Glass Button
                Rectangle {
                    width: 115
                    height: 36
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#0284c7" }
                        GradientStop { position: 1.0; color: "#38bdf8" }
                    }
                    border.color: saveMailMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "💾 Save Config"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: saveMailMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: authManager.saveMailConfig(adminSenderEmailField.text, adminApiKeyField.text)
                    }
                }
            }
        }

        // Modern Tab Bar Switcher
        Rectangle {
            Layout.fillWidth: true
            height: 44
            radius: 10
            color: "#0d1527"
            border.color: "#1d2e4a"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 6

                // Tab 0: Tenants
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: dashboardModal.activeTab === 0 ? "#0284c7" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { text: "🏢"; font.pixelSize: 14 }
                        Text {
                            text: "Corporate Tenants (" + dashboardModal.tenantsList.length + ")"
                            color: dashboardModal.activeTab === 0 ? "#ffffff" : "#94a3b8"
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: dashboardModal.activeTab = 0
                    }
                }

                // Tab 1: Solo Users
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: dashboardModal.activeTab === 1 ? "#0284c7" : "transparent"
                    Behavior on color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { text: "👤"; font.pixelSize: 14 }
                        Text {
                            text: "Solo Users (" + dashboardModal.soloUsersList.length + ")"
                            color: dashboardModal.activeTab === 1 ? "#ffffff" : "#94a3b8"
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: dashboardModal.activeTab = 1
                    }
                }

                // Search Filter
                TextField {
                    id: searchUsersField
                    placeholderText: dashboardModal.activeTab === 0 ? "🔍 Search tenant..." : "🔍 Search solo user..."
                    Layout.preferredWidth: 200
                    color: "#ffffff"
                    font.pixelSize: 11
                    background: Rectangle { color: "#162032"; radius: 6; border.color: "#273852" }
                    onTextChanged: dashboardModal.searchQuery = text.trim().toLowerCase()
                }
            }
        }

        // =========================================================================
        // TAB 0: CORPORATE TENANTS DIRECTORY VIEW
        // =========================================================================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: dashboardModal.activeTab === 0
            spacing: 12

            // Create New Tenant Bar
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 44
                radius: 8
                color: "#0f172a"
                border.color: "#1e293b"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    anchors.leftMargin: 12
                    spacing: 10

                    Text {
                        text: "➕ Create New Corporate Tenant:"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    TextField {
                        id: newTenantNameInput
                        placeholderText: "Tenant Name (e.g. Acme Corp, TechLabs)"
                        Layout.fillWidth: true
                        color: "#ffffff"
                        font.pixelSize: 12
                        background: Rectangle { color: "#162032"; radius: 6; border.color: "#273852" }
                    }

                    // Add Tenant Glass Button
                    Rectangle {
                        width: 110
                        height: 32
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#0284c7" }
                            GradientStop { position: 1.0; color: "#38bdf8" }
                        }
                        border.color: addTenantMouse.containsMouse ? "#ffffff" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "➕ Add Tenant"
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: addTenantMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (newTenantNameInput.text.trim().length > 0) {
                                    userManager.createTenant(newTenantNameInput.text.trim());
                                    newTenantNameInput.text = "";
                                    dashboardModal.refreshData();
                                }
                            }
                        }
                    }
                }
            }

            // Tenants Grid View
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                GridView {
                    id: tenantGrid
                    anchors.fill: parent
                    cellWidth: 280
                    cellHeight: 185
                    model: dashboardModal.tenantsList.filter(function(t) {
                        if (dashboardModal.searchQuery === "") return true;
                        return t.tenantId.toLowerCase().indexOf(dashboardModal.searchQuery) !== -1;
                    })

                    delegate: Rectangle {
                        id: tenantCard
                        width: 264
                        height: 170
                        radius: 12
                        color: tenantCardMouse.containsMouse ? "#1a2638" : "#111c2e"
                        border.color: tenantCardMouse.containsMouse ? "#38bdf8" : "#24354a"
                        border.width: tenantCardMouse.containsMouse ? 1.5 : 1

                        // Hover lift animation
                        transform: Translate { y: tenantCardMouse.containsMouse ? -3 : 0 }
                        Behavior on transform { NumberAnimation { duration: 150 } }
                        Behavior on color { ColorAnimation { duration: 150 } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        MouseArea {
                            id: tenantCardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Rectangle {
                                    width: 36
                                    height: 36
                                    radius: 10
                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: "#0284c7" }
                                        GradientStop { position: 1.0; color: "#0369a1" }
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: "🏢"
                                        font.pixelSize: 18
                                    }
                                }

                                ColumnLayout {
                                    spacing: 1
                                    Layout.fillWidth: true
                                    Text {
                                        text: modelData.tenantId
                                        color: "#f8fafc"
                                        font.pixelSize: 15
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: modelData.userCount + " Registered Sub-Users"
                                        color: "#38bdf8"
                                        font.pixelSize: 11
                                    }
                                }
                            }

                            // Visual Sub-User Capacity Bar
                            Rectangle {
                                Layout.fillWidth: true
                                height: 4
                                radius: 2
                                color: "#1e293b"

                                Rectangle {
                                    width: parent.width * Math.min(1.0, (modelData.userCount / 10.0))
                                    height: parent.height
                                    radius: 2
                                    color: "#38bdf8"
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: "⭐ Pro: " + modelData.proCount; color: "#f59e0b"; font.pixelSize: 11; font.bold: true }
                                Item { Layout.fillWidth: true }
                                Text { text: "🎬 Exports: " + modelData.exportsCount; color: "#10b981"; font.pixelSize: 11; font.bold: true }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                // View Users Glass Pill Button
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 30
                                    radius: 6
                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: "#0284c7" }
                                        GradientStop { position: 1.0; color: "#38bdf8" }
                                    }
                                    border.color: viewUsersMouse.containsMouse ? "#ffffff" : "transparent"
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: "👥 View Users"
                                        color: "#ffffff"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }

                                    MouseArea {
                                        id: viewUsersMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardModal.viewTenantUsers(modelData.tenantId)
                                    }
                                }

                                // Limits Glass Pill Button
                                Rectangle {
                                    Layout.preferredWidth: 85
                                    height: 30
                                    radius: 6
                                    color: "#1e293b"
                                    border.color: limitsMouse.containsMouse ? "#38bdf8" : "#334155"
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: "⚙ Limits"
                                        color: "#f8fafc"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }

                                    MouseArea {
                                        id: limitsMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardModal.configureTenantLimits(modelData.tenantId)
                                    }
                                }
                            }
                        }
                    }

                    // Empty State
                    Text {
                        anchors.centerIn: parent
                        visible: tenantGrid.count === 0
                        text: "No corporate tenants found. Create one above!"
                        color: "#64748b"
                        font.pixelSize: 13
                    }
                }
            }
        }

        // =========================================================================
        // TAB 1: SOLO USERS DIRECTORY VIEW
        // =========================================================================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: dashboardModal.activeTab === 1
            spacing: 10

            // Configure Global Solo Policy Card
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 46
                radius: 8
                color: "#0f172a"
                border.color: "#f59e0b"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 10

                    Text {
                        text: "⚙ Global Solo User Export Limits Policy:"
                        color: "#f59e0b"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    // Configure Solo Policy Glass Button
                    Rectangle {
                        width: 200
                        height: 32
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#d97706" }
                            GradientStop { position: 1.0; color: "#f59e0b" }
                        }
                        border.color: cfgSoloMouse.containsMouse ? "#ffffff" : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "⚙ Configure Solo Policy Limits"
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: cfgSoloMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dashboardModal.configureSoloPolicy()
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#0d1527"
                border.color: "#1d2e4a"
                border.width: 1
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Table Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 36
                        color: "#162032"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text { text: "Solo User / Email"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 260 }
                            Text { text: "Plan Tier"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 100 }
                            Text { text: "Exports"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 80 }
                            Text { text: "Registered Date"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 120 }
                            Text { text: "Actions"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.fillWidth: true }
                        }
                    }

                    // Table List View
                    ListView {
                        id: soloUserListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: dashboardModal.soloUsersList.filter(function(user) {
                            if (dashboardModal.searchQuery === "") return true;
                            return user.email.toLowerCase().indexOf(dashboardModal.searchQuery) !== -1 ||
                                   user.username.toLowerCase().indexOf(dashboardModal.searchQuery) !== -1;
                        })

                        delegate: Rectangle {
                            width: soloUserListView.width
                            height: 48
                            color: index % 2 === 0 ? "transparent" : "#0f1a2e"

                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 1
                                color: "#1d2e4a"
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12

                                // User Email & Name
                                RowLayout {
                                    Layout.preferredWidth: 260
                                    spacing: 8
                                    Rectangle {
                                        width: 28
                                        height: 28
                                        radius: 14
                                        color: modelData.tier === 2 ? "#f59e0b" : "#0284c7"
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.username.length > 0 ? modelData.username.charAt(0).toUpperCase() : "U"
                                            color: modelData.tier === 2 ? "#000000" : "#ffffff"
                                            font.bold: true
                                            font.pixelSize: 12
                                        }
                                    }
                                    ColumnLayout {
                                        spacing: 1
                                        Text { text: modelData.username; color: "#f8fafc"; font.pixelSize: 12; font.bold: true }
                                        Text { text: modelData.email; color: "#94a3b8"; font.pixelSize: 10; elide: Text.ElideRight; Layout.maximumWidth: 180 }
                                    }
                                }

                                // Plan Tier Tag
                                Rectangle {
                                    Layout.preferredWidth: 80
                                    height: 22
                                    radius: 11
                                    color: modelData.tier === 2 ? "#f59e0b" : "#0284c7"
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.tierName.toUpperCase()
                                        color: modelData.tier === 2 ? "#000000" : "#ffffff"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }

                                // Total Exports
                                Text { text: modelData.totalExports + " vids"; color: "#cbd5e1"; font.pixelSize: 11; Layout.preferredWidth: 80 }

                                // Registered Date
                                Text {
                                    text: modelData.registeredAt.length > 10 ? modelData.registeredAt.substring(0, 10) : modelData.registeredAt
                                    color: "#94a3b8"
                                    font.pixelSize: 10
                                    Layout.preferredWidth: 120
                                }

                                // Actions
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    // Upgrade / Tier Pill
                                    Rectangle {
                                        width: 90
                                        height: 28
                                        radius: 6
                                        gradient: Gradient {
                                            GradientStop { position: 0.0; color: modelData.tier === 2 ? "#334155" : "#0284c7" }
                                            GradientStop { position: 1.0; color: modelData.tier === 2 ? "#475569" : "#38bdf8" }
                                        }
                                        border.color: tierSoloMouse.containsMouse ? "#ffffff" : "transparent"
                                        border.width: 1

                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.tier === 2 ? "Set Free" : "Upgrade PRO"
                                            color: "#ffffff"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }

                                        MouseArea {
                                            id: tierSoloMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                if (typeof userManager !== "undefined") {
                                                    userManager.setUserTier(modelData.email, modelData.tier === 2 ? 1 : 2);
                                                }
                                            }
                                        }
                                    }

                                    // Delete User Pill
                                    Rectangle {
                                        width: 65
                                        height: 28
                                        radius: 6
                                        color: "#1e293b"
                                        border.color: delSoloMouse.containsMouse ? "#ef4444" : "#334155"
                                        border.width: 1

                                        Text {
                                            anchors.centerIn: parent
                                            text: "Delete"
                                            color: "#ef4444"
                                            font.pixelSize: 10
                                            font.bold: true
                                        }

                                        MouseArea {
                                            id: delSoloMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                if (typeof userManager !== "undefined") {
                                                    userManager.deleteUser(modelData.email);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Empty State
                        Text {
                            anchors.centerIn: parent
                            visible: soloUserListView.count === 0
                            text: "No solo users found in database."
                            color: "#64748b"
                            font.pixelSize: 13
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // POPUP 1: TENANT SUB-USERS DRILLDOWN MODAL
    // =========================================================================
    Popup {
        id: tenantUsersModal
        width: 740
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: Overlay.overlay

        background: Rectangle {
            color: "#0b1329"
            radius: 16
            border.color: "#38bdf8"
            border.width: 1.5
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: "🏢 Sub-Users Under Tenant: "
                    color: "#94a3b8"
                    font.pixelSize: 16
                }

                Text {
                    text: dashboardModal.activeTenantName
                    color: "#38bdf8"
                    font.pixelSize: 18
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                // Drilldown Modal Close Button
                Rectangle {
                    width: 80
                    height: 32
                    radius: 6
                    color: "#1e293b"
                    border.color: closeDrillMouse.containsMouse ? "#ef4444" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "✕ Close"
                        color: "#f8fafc"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    MouseArea {
                        id: closeDrillMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tenantUsersModal.close()
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#1e293b" }

            // Table Header
            Rectangle {
                Layout.fillWidth: true
                height: 34
                color: "#162032"
                radius: 6

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10

                    Text { text: "Sub-User / Email"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 260 }
                    Text { text: "Plan Tier"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 90 }
                    Text { text: "Exports"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 80 }
                    Text { text: "Actions"; color: "#94a3b8"; font.pixelSize: 11; font.bold: true; Layout.fillWidth: true }
                }
            }

            // Sub-Users List
            ListView {
                id: tenantUserListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: dashboardModal.selectedTenantUsersList

                delegate: Rectangle {
                    width: tenantUserListView.width
                    height: 48
                    color: index % 2 === 0 ? "transparent" : "#0f1a2e"
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10

                        // User Info
                        RowLayout {
                            Layout.preferredWidth: 260
                            spacing: 8
                            Rectangle {
                                width: 28
                                height: 28
                                radius: 14
                                color: modelData.tier === 2 ? "#f59e0b" : "#0284c7"
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.username.length > 0 ? modelData.username.charAt(0).toUpperCase() : "U"
                                    color: modelData.tier === 2 ? "#000000" : "#ffffff"
                                    font.bold: true
                                    font.pixelSize: 11
                                }
                            }
                            ColumnLayout {
                                spacing: 1
                                Text { text: modelData.username; color: "#f8fafc"; font.pixelSize: 12; font.bold: true }
                                Text { text: modelData.email; color: "#94a3b8"; font.pixelSize: 10; elide: Text.ElideRight; Layout.maximumWidth: 180 }
                            }
                        }

                        // Tier
                        Rectangle {
                            Layout.preferredWidth: 70
                            height: 20
                            radius: 10
                            color: modelData.tier === 2 ? "#f59e0b" : "#0284c7"
                            Text {
                                anchors.centerIn: parent
                                text: modelData.tierName.toUpperCase()
                                color: modelData.tier === 2 ? "#000000" : "#ffffff"
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }

                        // Exports
                        Text { text: modelData.totalExports + " vids"; color: "#cbd5e1"; font.pixelSize: 11; Layout.preferredWidth: 80 }

                        // Actions
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            // Upgrade PRO / Set Free Pill
                            Rectangle {
                                width: 85
                                height: 28
                                radius: 6
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: modelData.tier === 2 ? "#334155" : "#0284c7" }
                                    GradientStop { position: 1.0; color: modelData.tier === 2 ? "#475569" : "#38bdf8" }
                                }
                                border.color: tierDrillMouse.containsMouse ? "#ffffff" : "transparent"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.tier === 2 ? "Set Free" : "Upgrade PRO"
                                    color: "#ffffff"
                                    font.pixelSize: 10
                                    font.bold: true
                                }

                                MouseArea {
                                    id: tierDrillMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof userManager !== "undefined") {
                                            userManager.setUserTier(modelData.email, modelData.tier === 2 ? 1 : 2);
                                        }
                                    }
                                }
                            }

                            // Remove from Tenant Pill
                            Rectangle {
                                width: 125
                                height: 28
                                radius: 6
                                color: "#1e293b"
                                border.color: rmDrillMouse.containsMouse ? "#ef4444" : "#334155"
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "Remove from Tenant"
                                    color: "#f8fafc"
                                    font.pixelSize: 10
                                    font.bold: true
                                }

                                MouseArea {
                                    id: rmDrillMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (typeof userManager !== "undefined") {
                                            userManager.setUserTenant(modelData.email, "main_tenant");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: tenantUserListView.count === 0
                    text: "No sub-users assigned to this tenant yet."
                    color: "#64748b"
                    font.pixelSize: 12
                }
            }
        }
    }

    // =========================================================================
    // POPUP 2: CONFIGURE TENANT EXPORT LIMITS MODAL (WITH PRESET QUICK CHIPS)
    // =========================================================================
    Popup {
        id: tenantLimitsModal
        width: 660
        height: 530
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: Overlay.overlay

        background: Rectangle {
            color: "#0b1329"
            radius: 16
            border.color: "#38bdf8"
            border.width: 1.5
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 22
            spacing: 16

            Text {
                text: "⚙ Configure Custom Export Limits: " + dashboardModal.activeTenantName
                color: "#f8fafc"
                font.pixelSize: 18
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#1e293b" }

            // Preset Chips Flow
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text { text: "Quick Preset Configurations:"; color: "#38bdf8"; font.pixelSize: 11; font.bold: true }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    // Chip 1: Standard
                    Rectangle {
                        implicitWidth: stdText.implicitWidth + 24
                        height: 32
                        radius: 8
                        color: "#1e293b"
                        border.color: stdMouse.containsMouse ? "#38bdf8" : "#334155"
                        border.width: 1

                        Text {
                            id: stdText
                            anchors.centerIn: parent
                            text: "Standard (5 Exp / 5 Min / 1080p)"
                            color: "#f8fafc"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: stdMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dashboardModal.applyPresetToTenant("5", "5", 0)
                        }
                    }

                    // Chip 2: Business
                    Rectangle {
                        implicitWidth: bizText.implicitWidth + 24
                        height: 32
                        radius: 8
                        color: "#1e293b"
                        border.color: bizMouse.containsMouse ? "#38bdf8" : "#334155"
                        border.width: 1

                        Text {
                            id: bizText
                            anchors.centerIn: parent
                            text: "Business (25 Exp / 30 Min / 1440p)"
                            color: "#f8fafc"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: bizMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dashboardModal.applyPresetToTenant("25", "30", 1)
                        }
                    }

                    // Chip 3: Unlimited
                    Rectangle {
                        implicitWidth: unlText.implicitWidth + 24
                        height: 32
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#0284c7" }
                            GradientStop { position: 1.0; color: "#38bdf8" }
                        }
                        border.color: unlMouse.containsMouse ? "#ffffff" : "transparent"
                        border.width: 1

                        Text {
                            id: unlText
                            anchors.centerIn: parent
                            text: "Unlimited 4K (∞)"
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: unlMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dashboardModal.applyPresetToTenant("-1", "-1", 2)
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Daily Export Count Limit (-1 for Unlimited):"; color: "#cbd5e1"; font.pixelSize: 12 }
                TextField {
                    id: tenantLimitDailyInput
                    Layout.fillWidth: true
                    implicitHeight: 38
                    placeholderText: "e.g. 10 (-1 = Unlimited)"
                    placeholderTextColor: "#64748b"
                    color: "#f8fafc"
                    font.pixelSize: 13
                    background: Rectangle { color: "#162032"; radius: 8; border.color: tenantLimitDailyInput.activeFocus ? "#38bdf8" : "#273852" }
                }
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Max Video Export Duration in Minutes (-1 for Unlimited):"; color: "#cbd5e1"; font.pixelSize: 12 }
                TextField {
                    id: tenantLimitDurationInput
                    Layout.fillWidth: true
                    implicitHeight: 38
                    placeholderText: "e.g. 15 (-1 = Unlimited)"
                    placeholderTextColor: "#64748b"
                    color: "#f8fafc"
                    font.pixelSize: 13
                    background: Rectangle { color: "#162032"; radius: 8; border.color: tenantLimitDurationInput.activeFocus ? "#38bdf8" : "#273852" }
                }
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Max Allowed Resolution Quality:"; color: "#cbd5e1"; font.pixelSize: 12 }
                ComboBox {
                    id: tenantLimitResCombo
                    Layout.fillWidth: true
                    implicitHeight: 38
                    model: ["1080p (Full HD)", "1440p (2K Quad HD)", "2160p (4K Ultra HD)"]
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // Cancel Button
                Rectangle {
                    width: 100
                    height: 38
                    radius: 8
                    color: "#1e293b"
                    border.color: cancelTenantMouse.containsMouse ? "#38bdf8" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelTenantMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tenantLimitsModal.close()
                    }
                }

                Item { Layout.fillWidth: true }

                // Save Button
                Rectangle {
                    width: 170
                    height: 38
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#0284c7" }
                        GradientStop { position: 1.0; color: "#38bdf8" }
                    }
                    border.color: saveTenantMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "💾 Save Tenant Limits"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: saveTenantMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof userManager !== "undefined") {
                                var daily = parseInt(tenantLimitDailyInput.text) || 10;
                                var dur = parseInt(tenantLimitDurationInput.text) || 10;
                                var res = (tenantLimitResCombo.currentIndex === 2) ? 2160 : ((tenantLimitResCombo.currentIndex === 1) ? 1440 : 1080);
                                userManager.setTenantLimits(dashboardModal.activeTenantName, daily, dur, res);
                                if (typeof quotaManager !== "undefined") {
                                    quotaManager.quotaChanged();
                                }
                                tenantLimitsModal.close();
                            }
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // POPUP 3: CONFIGURE GLOBAL SOLO POLICY MODAL
    // =========================================================================
    Popup {
        id: soloLimitsModal
        width: 580
        height: 440
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: Overlay.overlay

        background: Rectangle {
            color: "#0b1329"
            radius: 16
            border.color: "#f59e0b"
            border.width: 1.5
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 22
            spacing: 16

            Text {
                text: "⚙ Configure Global Solo User Export Limits Policy"
                color: "#f59e0b"
                font.pixelSize: 18
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#1e293b" }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Global Solo Daily Export Limit (-1 for Unlimited):"; color: "#cbd5e1"; font.pixelSize: 12 }
                TextField {
                    id: soloLimitDailyInput
                    Layout.fillWidth: true
                    implicitHeight: 38
                    placeholderText: "e.g. 2 (-1 = Unlimited)"
                    placeholderTextColor: "#64748b"
                    color: "#f8fafc"
                    font.pixelSize: 13
                    background: Rectangle { color: "#162032"; radius: 8; border.color: soloLimitDailyInput.activeFocus ? "#f59e0b" : "#273852" }
                }
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Global Solo Max Video Duration in Minutes (-1 for Unlimited):"; color: "#cbd5e1"; font.pixelSize: 12 }
                TextField {
                    id: soloLimitDurationInput
                    Layout.fillWidth: true
                    implicitHeight: 38
                    placeholderText: "e.g. 2 (-1 = Unlimited)"
                    placeholderTextColor: "#64748b"
                    color: "#f8fafc"
                    font.pixelSize: 13
                    background: Rectangle { color: "#162032"; radius: 8; border.color: soloLimitDurationInput.activeFocus ? "#f59e0b" : "#273852" }
                }
            }

            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Text { text: "Global Solo Max Resolution Quality:"; color: "#cbd5e1"; font.pixelSize: 12 }
                ComboBox {
                    id: soloLimitResCombo
                    Layout.fillWidth: true
                    implicitHeight: 38
                    model: ["1080p (Full HD)", "1440p (2K Quad HD)", "2160p (4K Ultra HD)"]
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // Cancel Button
                Rectangle {
                    width: 100
                    height: 38
                    radius: 8
                    color: "#1e293b"
                    border.color: cancelSoloMouse.containsMouse ? "#f59e0b" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: "#f8fafc"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelSoloMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: soloLimitsModal.close()
                    }
                }

                Item { Layout.fillWidth: true }

                // Save Button
                Rectangle {
                    width: 180
                    height: 38
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#d97706" }
                        GradientStop { position: 1.0; color: "#f59e0b" }
                    }
                    border.color: saveSoloMouse.containsMouse ? "#ffffff" : "transparent"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "💾 Save Global Policy"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                    }

                    MouseArea {
                        id: saveSoloMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof userManager !== "undefined") {
                                var daily = parseInt(soloLimitDailyInput.text) || 2;
                                var dur = parseInt(soloLimitDurationInput.text) || 2;
                                var res = (soloLimitResCombo.currentIndex === 2) ? 2160 : ((soloLimitResCombo.currentIndex === 1) ? 1440 : 1080);
                                userManager.setSoloPolicy(daily, dur, res);
                                if (typeof quotaManager !== "undefined") {
                                    quotaManager.quotaChanged();
                                }
                                soloLimitsModal.close();
                            }
                        }
                    }
                }
            }
        }
    }
}
