qt_add_executable(hyprpdf
    main.cpp
)

target_link_libraries(hyprpdf PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    $<$<TARGET_EXISTS:Qt6::Pdf>:Qt6::Pdf>
    Qt6::Svg
    Qt6::Widgets
    Qt6::DBus
)

qt_add_qml_module(hyprpdf
    URI HyprPDF
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/PdfView.qml
    RESOURCE_PREFIX /
)

install(TARGETS hyprpdf RUNTIME DESTINATION bin)
