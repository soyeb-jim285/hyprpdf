pragma Singleton
import QtQuick

QtObject {
    property color base:    theme.base
    property color mantle:  theme.mantle
    property color crust:   theme.crust
    property color surface: theme.surface
    property color overlay: theme.overlay
    property color text:    theme.text
    property color subtext: theme.subtext
    property color muted:   theme.muted
    property color accent:  theme.accent
    property color success: theme.success
    property color warning: theme.warning
    property color error:   theme.error

    property int radiusSmall:  config.radiusSm
    property int radiusMedium: config.radiusMd
    property int radiusLarge:  config.radiusLg

    readonly property int spacing: 8
    readonly property int fontSmall: 11
    readonly property int fontNormal: 13
    readonly property int fontLarge: 16
    readonly property int controlSize: 32
    readonly property int toolbarRowHeight: 40

    property bool animationsEnabled: config.animationsEnabled
    readonly property int animDurationFast: animationsEnabled ? config.animDurationFast   : 0
    readonly property int animDuration:     animationsEnabled ? config.animDurationNormal : 0
    readonly property int animDurationSlow: animationsEnabled ? config.animDurationSlow   : 0

    property bool transparencyEnabled: config.transparencyEnabled
    property real transparencyLevel:   config.transparencyLevel

    function annot(name) { return theme.annotation(name) }
    function containerColor(c, a) {
        return Qt.rgba(c.r, c.g, c.b,
                       transparencyEnabled ? a * transparencyLevel : 1)
    }
}
