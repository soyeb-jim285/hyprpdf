import QtQuick
import QtQuick.Shapes
import HyprPDF 1.0

Item {
    id: root
    property int pageIndex: 0
    property real pxPerPt: 1.0
    property var pageImage: null
    signal annotationClicked(string id, int type, point scenePt)

    Repeater {
        model: {
            annotationStore.revision
            return annotationStore.annotationsOnPage(root.pageIndex)
        }
        delegate: Loader {
            required property var modelData
            property var annot: modelData
            sourceComponent: {
                switch (annot.type) {
                    case 0: return highlightC
                    case 1: return underlineC
                    case 2: return strikeoutC
                    case 3: return stickyC
                    case 4: return inkC
                }
                return null
            }
        }
    }

    Component {
        id: highlightC
        Item {
            required property var annot
            Repeater {
                model: annot.rects
                delegate: Item {
                    required property var modelData
                    x: modelData.x * root.pxPerPt
                    y: modelData.y * root.pxPerPt
                    width:  modelData.width  * root.pxPerPt
                    height: modelData.height * root.pxPerPt
                    opacity: 0.55
                    ShaderEffectSource {
                        id: src
                        anchors.fill: parent
                        live: false
                        hideSource: false
                        sourceItem: root.pageImage
                        sourceRect: Qt.rect(parent.x, parent.y, parent.width, parent.height)
                    }
                    ShaderEffect {
                        anchors.fill: parent
                        property variant src: src
                        property color tintColor: annot.color
                        property vector2d boxSize: Qt.vector2d(width, height)
                        property real boxRadius: Math.min(4, height / 3)
                        property real tintStrength: 0.85
                        fragmentShader: "qrc:/HyprPDF/qml/shaders/highlight_under_text.frag.qsb"
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            const sp = mapToItem(null, m.x, m.y)
                            root.annotationClicked(annot.id, 0, Qt.point(sp.x, sp.y))
                        }
                    }
                }
            }
        }
    }

    Component {
        id: underlineC
        Item {
            required property var annot
            Repeater {
                model: annot.rects
                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x * root.pxPerPt
                    y: (modelData.y + modelData.height - 1.5) * root.pxPerPt
                    width: modelData.width * root.pxPerPt
                    height: 1.5
                    color: annot.color
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            const sp = mapToItem(null, m.x, m.y)
                            root.annotationClicked(annot.id, 1, Qt.point(sp.x, sp.y))
                        }
                    }
                }
            }
        }
    }

    Component {
        id: strikeoutC
        Item {
            required property var annot
            Repeater {
                model: annot.rects
                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x * root.pxPerPt
                    y: (modelData.y + modelData.height / 2) * root.pxPerPt
                    width: modelData.width * root.pxPerPt
                    height: 1.5
                    color: annot.color
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            const sp = mapToItem(null, m.x, m.y)
                            root.annotationClicked(annot.id, 2, Qt.point(sp.x, sp.y))
                        }
                    }
                }
            }
        }
    }

    Component {
        id: stickyC
        Item {
            required property var annot
            x: annot.anchor.x * root.pxPerPt - 12
            y: annot.anchor.y * root.pxPerPt - 12
            width: 24
            height: 24
            Rectangle {
                anchors.fill: parent
                radius: 4
                color: annot.color
                opacity: 0.9
                border.color: Theme.crust
                border.width: 1
            }
            MouseArea {
                anchors.fill: parent
                onClicked: (m) => {
                    const sp = mapToItem(null, m.x, m.y)
                    root.annotationClicked(annot.id, 3, Qt.point(sp.x, sp.y))
                }
            }
        }
    }

    Component {
        id: inkC
        Item {
            required property var annot
            anchors.fill: parent
            Repeater {
                model: annot.strokes
                delegate: Shape {
                    required property var modelData
                    anchors.fill: parent
                    preferredRendererType: Shape.CurveRenderer
                    ShapePath {
                        strokeColor: annot.color
                        strokeWidth: annot.strokeWidth
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap
                        joinStyle: ShapePath.RoundJoin
                        scale: Qt.size(root.pxPerPt, root.pxPerPt)
                        PathPolyline { path: modelData }
                    }
                }
            }
        }
    }
}
