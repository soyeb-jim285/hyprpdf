import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import HyprPDF 1.0

Rectangle {
    id: root
    implicitHeight: 44
    color: Theme.mantle

    signal openRequested()
    signal newTabRequested()
    signal closeTabRequested()
    signal zoomInRequested()
    signal zoomOutRequested()
    signal fitWidthRequested()
    signal fitPageRequested()
    signal searchRequested()
    signal invertToggled()
    signal leftPanelToggled()
    signal rightPanelToggled()
    signal settingsRequested()

    property bool invertOn: false
    property bool leftOn: true
    property bool rightOn: true

    // Fallback icon-button composition: Rectangle + HoverHandler + MouseArea + SVG icon + Tooltip
    component IBtn: Rectangle {
        id: ibtnRoot
        property string tipText: ""
        property bool btnEnabled: true
        property bool active: false
        signal clicked()

        implicitWidth: 34
        implicitHeight: 34
        radius: Theme.radiusMedium
        color: active ? Theme.surface
             : ihover.hovered ? Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.08)
             : "transparent"
        opacity: btnEnabled ? 1.0 : 0.45
        Behavior on color { ColorAnimation { duration: Theme.animDurationFast } }

        HoverHandler { id: ihover }
        MouseArea {
            anchors.fill: parent
            enabled: ibtnRoot.btnEnabled
            cursorShape: ibtnRoot.btnEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (ibtnRoot.btnEnabled) ibtnRoot.clicked()
        }
        Q.Tooltip {
            target: ibtnRoot
            text: ibtnRoot.tipText
            visible: ibtnRoot.tipText !== "" && ihover.hovered
        }
    }

    component VSep: Rectangle {
        implicitWidth: 1
        implicitHeight: 22
        Layout.alignment: Qt.AlignVCenter
        color: Qt.rgba(Theme.text.r, Theme.text.g, Theme.text.b, 0.15)
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 4

        // File group
        IBtn {
            tipText: "Open (Ctrl+O)"
            onClicked: root.openRequested()
            IconFolder { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "New tab (Ctrl+T)"
            onClicked: root.newTabRequested()
            IconPlus { size: 18; color: Theme.text; anchors.centerIn: parent }
        }

        VSep {}

        // Zoom group
        IBtn {
            tipText: "Zoom out (Ctrl+-)"
            onClicked: root.zoomOutRequested()
            IconZoomOut { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Zoom in (Ctrl++)"
            onClicked: root.zoomInRequested()
            IconZoomIn { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Fit width (Ctrl+1)"
            onClicked: root.fitWidthRequested()
            IconMaximize { size: 18; color: Theme.text; anchors.centerIn: parent }
        }

        VSep {}

        // Navigation panels group
        IBtn {
            tipText: "Toggle thumbnails (F9)"
            active: root.leftOn
            onClicked: root.leftPanelToggled()
            IconPanelLeft { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Toggle outline (F10)"
            active: root.rightOn
            onClicked: root.rightPanelToggled()
            IconPanelTop {
                size: 18; color: Theme.text; anchors.centerIn: parent
                rotation: 90
            }
        }

        VSep {}

        // Find + invert
        IBtn {
            tipText: "Find (Ctrl+F)"
            onClicked: root.searchRequested()
            IconSearch { size: 18; color: Theme.text; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Invert colors (Ctrl+I)"
            active: root.invertOn
            onClicked: root.invertToggled()
            Loader {
                anchors.centerIn: parent
                sourceComponent: root.invertOn ? sunIcon : moonIcon
            }
            Component { id: moonIcon; IconMoon { size: 18; color: Theme.text } }
            Component { id: sunIcon;  IconSun  { size: 18; color: Theme.text } }
        }

        Item { Layout.fillWidth: true }

        IBtn {
            tipText: "Settings (stub - v0.6)"
            btnEnabled: false
            onClicked: root.settingsRequested()
            IconSettings { size: 18; color: Theme.muted; anchors.centerIn: parent }
        }
    }
}
