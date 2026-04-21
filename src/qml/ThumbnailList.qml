import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Quill 1.0 as Q
import HyprPDF 1.0

Rectangle {
    id: root
    property var document: null       // PdfDoc or null
    property int currentPage: 0
    signal pageActivated(int page)

    color: Theme.mantle
    implicitWidth: 180

    Text {
        anchors.centerIn: parent
        visible: !root.document
        text: "No document"
        color: Theme.subtext
        opacity: 0.6
    }

    ListView {
        id: list
        anchors.fill: parent
        anchors.margins: 8
        model: root.document ? root.document.pageCount : 0
        spacing: 8
        clip: true
        cacheBuffer: 2000
        currentIndex: root.currentPage

        delegate: Column {
            width: list.width - 16
            spacing: 2

            Rectangle {
                width: parent.width
                readonly property real aspectH: {
                    if (!root.document) return width * 1.29
                    const s = root.document.pageSize(index)
                    return (s.width > 0) ? (width * (s.height / s.width)) : (width * 1.29)
                }
                height: aspectH
                color: "white"
                border.color: index === root.currentPage ? Theme.accent : Theme.surface
                border.width: index === root.currentPage ? 2 : 1

                Image {
                    anchors.fill: parent
                    anchors.margins: 1
                    source: root.document
                            ? "image://pdf/" + root.document.id + "/" + index + "/" + Math.round(parent.width)
                            : ""
                    asynchronous: true
                    cache: true
                    fillMode: Image.PreserveAspectFit
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.pageActivated(index)
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: (index + 1).toString()
                color: Theme.subtext
                font.pixelSize: Theme.fontSmall
            }
        }
    }
}
