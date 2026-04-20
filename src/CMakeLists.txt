qt_add_executable(hyprpdf
    main.cpp
    services/themeloader.cpp
    services/configmanager.cpp
    services/documentcontroller.cpp
    services/pdfdoc.cpp
    services/pdfpageimageprovider.cpp
    models/documentmodel.cpp
    models/recentfilesmodel.cpp
)

target_link_libraries(hyprpdf PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Widgets
    Qt6::DBus
    PkgConfig::POPPLER_QT6
)

target_include_directories(hyprpdf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(hyprpdf
    URI HyprPDF
    VERSION 1.0
    NO_CACHEGEN
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/PdfView.qml
        qml/Toolbar.qml
        qml/TabsBar.qml
        qml/RecentFilesView.qml
    RESOURCE_PREFIX /
)

install(TARGETS hyprpdf RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/
        DESTINATION share/hyprpdf/themes
        FILES_MATCHING PATTERN "*.toml")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/qml/Quill
        DESTINATION share/hyprpdf/src/qml)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/qml/icons
        DESTINATION share/hyprpdf/src/qml)
install(FILES ${CMAKE_SOURCE_DIR}/dist/hyprpdf.desktop
        DESTINATION share/applications)
