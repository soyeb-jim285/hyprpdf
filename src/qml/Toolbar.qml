import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import HyprPDF 1.0

Rectangle {
    id: root
    implicitHeight: Theme.toolbarRowHeight
    color: Theme.mantle

    signal openRequested()
    signal zoomInRequested()
    signal zoomOutRequested()
    signal fitPageRequested()

    // Fallback icon-button composition: Rectangle + HoverHandler + MouseArea + SVG icon + Tooltip
    component IBtn: Rectangle {
        id: ibtnRoot
        property string tipText: ""
        property bool btnEnabled: true
        signal clicked()

        implicitWidth: 34; implicitHeight: 34
        radius: Q.Theme.radiusSm
        color: ihover.hovered
               ? Q.Theme.surface0
               : "transparent"
        opacity: btnEnabled ? 1.0 : 0.45
        Behavior on color { ColorAnimation { duration: Q.Theme.animDurationFast } }

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

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing
        anchors.rightMargin: Theme.spacing
        spacing: 4

        IBtn {
            tipText: "Open (Ctrl+O)"
            onClicked: root.openRequested()
            IconFolder { size: 18; color: Q.Theme.textPrimary; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Zoom in (Ctrl++)"
            onClicked: root.zoomInRequested()
            IconZoomIn { size: 18; color: Q.Theme.textPrimary; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Zoom out (Ctrl+-)"
            onClicked: root.zoomOutRequested()
            IconZoomOut { size: 18; color: Q.Theme.textPrimary; anchors.centerIn: parent }
        }
        IBtn {
            tipText: "Fit page (Ctrl+2)"
            onClicked: root.fitPageRequested()
            IconMaximize { size: 18; color: Q.Theme.textPrimary; anchors.centerIn: parent }
        }

        Item { Layout.fillWidth: true }

        IBtn {
            tipText: "Settings (stub — v0.6)"
            btnEnabled: false
            IconSettings { size: 18; color: Q.Theme.textTertiary; anchors.centerIn: parent }
        }
    }
}
