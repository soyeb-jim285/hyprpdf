import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Quill 1.0 as Q
import HyprPDF 1.0

Rectangle {
    id: root
    property var controller: null
    property alias text: field.text

    width: 420
    height: 40
    color: Theme.surface
    border.color: Theme.overlay
    border.width: 1
    radius: Theme.radiusMedium
    visible: false

    signal closed()

    function openWithFocus() {
        visible = true
        field.forceActiveFocus()
        field.selectAll()
    }

    Timer {
        id: debounce
        interval: 150
        onTriggered: if (root.controller) root.controller.search(field.text)
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        TextField {
            id: field
            Layout.fillWidth: true
            placeholderText: "Find in document"
            color: Theme.text
            background: Rectangle { color: "transparent" }
            onTextEdited: debounce.restart()
            Keys.onReturnPressed: if (root.controller) root.controller.next()
            Keys.onEscapePressed: { root.visible = false; root.closed() }
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            visible: root.controller && root.controller.matchCount > 0
            text: (root.controller.currentIndex + 1) + " / " + root.controller.matchCount
            color: Theme.subtext
            font.pixelSize: Theme.fontSmall
        }
        Text {
            Layout.alignment: Qt.AlignVCenter
            visible: field.text.length > 0 && root.controller && root.controller.matchCount === 0
            text: "none"
            color: Theme.error
            font.pixelSize: Theme.fontSmall
        }

        IconChevronUp {
            implicitWidth: 20; implicitHeight: 20
            size: 16
            color: Theme.text
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: if (root.controller) root.controller.prev()
            }
        }
        IconChevronDown {
            implicitWidth: 20; implicitHeight: 20
            size: 16
            color: Theme.text
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: if (root.controller) root.controller.next()
            }
        }
        IconX {
            implicitWidth: 20; implicitHeight: 20
            size: 16
            color: Theme.text
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: { root.visible = false; root.closed() }
            }
        }
    }
}
