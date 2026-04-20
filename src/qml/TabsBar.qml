import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import Icons 1.0 as Ico
import HyprPDF 1.0

Rectangle {
    id: root
    visible: documentModel.count > 0
    implicitHeight: visible ? 32 : 0
    color: Theme.crust

    signal openRequested()

    // Fallback icon-button composition
    component IBtn: Rectangle {
        id: tbBtnRoot
        property string tipText: ""
        signal clicked()

        implicitWidth: 22; implicitHeight: 22
        radius: Q.Theme.radiusSm
        color: tbhover.hovered ? Q.Theme.surface0 : "transparent"
        Behavior on color { ColorAnimation { duration: Q.Theme.animDurationFast } }

        HoverHandler { id: tbhover }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: tbBtnRoot.clicked()
        }
        Q.Tooltip {
            target: tbBtnRoot
            text: tbBtnRoot.tipText
            visible: tbBtnRoot.tipText !== "" && tbhover.hovered
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: documentModel
            delegate: Rectangle {
                Layout.preferredWidth: 180
                Layout.fillHeight: true
                color: index === documentModel.currentIndex ? Theme.surface : Theme.mantle
                border.color: Theme.crust
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    onClicked: documentModel.currentIndex = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 4
                    spacing: 6

                    Text {
                        Layout.fillWidth: true
                        text: model.title !== undefined ? model.title : ""
                        color: Theme.text
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Theme.fontSmall
                    }

                    IBtn {
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                        implicitWidth: 18; implicitHeight: 18
                        tipText: "Close tab"
                        onClicked: documentModel.closeTab(index)
                        Ico.IconX { size: 12; color: Q.Theme.textSecondary; anchors.centerIn: parent }
                    }
                }
            }
        }

        // "New tab / open" button
        Rectangle {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignVCenter
            color: newHover.hovered ? Q.Theme.surface0 : "transparent"
            radius: Q.Theme.radiusSm
            Behavior on color { ColorAnimation { duration: Q.Theme.animDurationFast } }

            HoverHandler { id: newHover }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.openRequested()
            }
            Q.Tooltip {
                target: parent
                text: "Open another document (Ctrl+T)"
                visible: newHover.hovered
            }
            Ico.IconPlus { size: 14; color: Q.Theme.textPrimary; anchors.centerIn: parent }
        }

        Item { Layout.fillWidth: true }
    }
}
