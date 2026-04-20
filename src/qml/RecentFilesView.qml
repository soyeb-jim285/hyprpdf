import QtQuick
import QtQuick.Layouts
import Quill 1.0 as Q
import HyprPDF 1.0

Item {
    id: root
    signal openRequested()

    // Use recentFiles.count (QAbstractListModel exposes count to QML)
    readonly property int recentCount: recentFiles.rowCount()

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(640, parent.width - 64)
        spacing: 16

        Text {
            text: "HyprPDF"
            color: Theme.text
            font.pixelSize: 28
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            visible: root.recentCount === 0
            text: "No recent documents.  Press Ctrl+O to open one."
            color: Theme.subtext
            Layout.alignment: Qt.AlignHCenter
        }

        Q.Button {
            visible: root.recentCount === 0
            text: "Open document..."
            onClicked: root.openRequested()
            Layout.alignment: Qt.AlignHCenter
        }

        Q.ScrollableList {
            visible: root.recentCount > 0
            Layout.fillWidth: true
            Layout.preferredHeight: 360
            model: recentFiles
            delegate: Q.Card {
                width: ListView.view ? ListView.view.width : 400
                opacity: model.exists ? 1.0 : 0.55

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: recentFiles.open(index)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Loader {
                        sourceComponent: {
                            if (model.iconName === "IconLayoutGrid") return layoutGrid
                            if (model.iconName === "IconBookOpen")   return bookOpen
                            return fileText
                        }
                    }

                    Component { id: fileText;   IconFileText   { size: 24; color: Q.Theme.textPrimary } }
                    Component { id: layoutGrid; IconLayoutGrid { size: 24; color: Q.Theme.textPrimary } }
                    Component { id: bookOpen;   IconBookOpen   { size: 24; color: Q.Theme.textPrimary } }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: model.name !== undefined ? model.name : ""
                            color: Theme.text
                            font.pixelSize: Theme.fontNormal
                        }
                        Text {
                            text: model.path !== undefined ? model.path : ""
                            color: Theme.subtext
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
