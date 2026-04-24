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
            anchors.fill: parent
            required property var modelData
            sourceComponent: {
                if (!modelData) return null
                switch (modelData.type) {
                    case 0: return highlightC
                    case 1: return underlineC
                    case 2: return strikeoutC
                    case 3: return stickyC
                    case 4: return inkC
                    case 5: return textBoxC
                }
                return null
            }
            onLoaded: if (item) item.annot = modelData
        }
    }

    Component {
        id: highlightC
        Item {
            property var annot: null
            Repeater {
                model: annot ? annot.rects : []
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
                        live: true
                        hideSource: false
                        sourceItem: root.pageImage
                        sourceRect: Qt.rect(parent.x, parent.y, parent.width, parent.height)
                    }
                    ShaderEffect {
                        anchors.fill: parent
                        property variant src: src
                        property color tintColor: annot ? annot.color : "#f9e2af"
                        property vector2d boxSize: Qt.vector2d(width, height)
                        property real boxRadius: Math.min(4, height / 3)
                        property real tintStrength: 0.85
                        fragmentShader: "qrc:/HyprPDF/qml/shaders/highlight_under_text.frag.qsb"
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            if (!annot) return
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
            property var annot: null
            Repeater {
                model: annot ? annot.rects : []
                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x * root.pxPerPt
                    y: (modelData.y + modelData.height) * root.pxPerPt - 2
                    width: modelData.width * root.pxPerPt
                    height: 2
                    color: annot ? annot.color : "transparent"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            if (!annot) return
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
            property var annot: null
            Repeater {
                model: annot ? annot.rects : []
                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x * root.pxPerPt
                    y: (modelData.y + modelData.height / 2) * root.pxPerPt - 1
                    width: modelData.width * root.pxPerPt
                    height: 2
                    color: annot ? annot.color : "transparent"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: (m) => {
                            if (!annot) return
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
            property var annot: null
            x: annot ? (annot.anchor.x * root.pxPerPt - 12) : 0
            y: annot ? (annot.anchor.y * root.pxPerPt - 12) : 0
            width: 24
            height: 24
            visible: !!annot
            Rectangle {
                anchors.fill: parent
                radius: 4
                color: annot ? annot.color : "transparent"
                opacity: 0.9
                border.color: Theme.crust
                border.width: 1
            }
            MouseArea {
                anchors.fill: parent
                onClicked: (m) => {
                    if (!annot) return
                    const sp = mapToItem(null, m.x, m.y)
                    root.annotationClicked(annot.id, 3, Qt.point(sp.x, sp.y))
                }
            }
        }
    }

    Component {
        id: inkC
        Item {
            id: inkRoot
            property var annot: null
            anchors.fill: parent
            onAnnotChanged: console.log("inkC annot set strokes=" + (annot ? annot.strokes.length : "null") + " w=" + width + " h=" + height + " pxPerPt=" + root.pxPerPt)
            Repeater {
                model: annot ? annot.strokes : []
                delegate: Shape {
                    id: stroke
                    required property var modelData
                    anchors.fill: parent
                    preferredRendererType: Shape.GeometryRenderer
                    Component.onCompleted: {
                        const len = modelData ? modelData.length : -1
                        const first = (modelData && modelData[0]) ? (modelData[0].x + "," + modelData[0].y) : "?"
                        console.log("ink shape: pts=" + len + " first=" + first
                                    + " color=" + (annot ? annot.color : "null")
                                    + " w=" + width + " h=" + height)
                    }
                    ShapePath {
                        strokeColor: annot ? annot.color : "#000000"
                        strokeWidth: annot ? annot.strokeWidth : 2
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap
                        joinStyle: ShapePath.RoundJoin
                        startX: stroke.modelData && stroke.modelData.length > 0
                                ? stroke.modelData[0].x * root.pxPerPt : 0
                        startY: stroke.modelData && stroke.modelData.length > 0
                                ? stroke.modelData[0].y * root.pxPerPt : 0
                        PathSvg {
                            path: {
                                const pts = stroke.modelData
                                if (!pts || pts.length < 2) return ""
                                let s = ""
                                for (let i = 1; i < pts.length; ++i) {
                                    s += " L " + (pts[i].x * root.pxPerPt)
                                              + " " + (pts[i].y * root.pxPerPt)
                                }
                                return s
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: textBoxC
        Item {
            property var annot: null
            x: annot ? annot.anchor.x * root.pxPerPt : 0
            y: annot ? annot.anchor.y * root.pxPerPt : 0
            visible: !!annot
            Text {
                id: label
                text: annot ? (annot.note || "") : ""
                color: annot ? annot.color : "#000000"
                font.pixelSize: annot ? annot.fontSize * root.pxPerPt : 14
            }
            MouseArea {
                anchors.fill: label
                onClicked: (m) => {
                    if (!annot) return
                    const sp = mapToItem(null, m.x, m.y)
                    root.annotationClicked(annot.id, 5, Qt.point(sp.x, sp.y))
                }
            }
        }
    }
}
