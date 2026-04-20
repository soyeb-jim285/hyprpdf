function(hyprpdf_test name)
    qt_add_executable(${name} ${name}.cpp ${ARGN})
    target_link_libraries(${name} PRIVATE Qt6::Test Qt6::Core Qt6::Gui)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_compile_definitions(${name} PRIVATE
        HYPRPDF_THEMES_DIR="${CMAKE_SOURCE_DIR}/themes"
        HYPRPDF_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

hyprpdf_test(tst_themeloader ${CMAKE_SOURCE_DIR}/src/services/themeloader.cpp)
hyprpdf_test(tst_configmanager ${CMAKE_SOURCE_DIR}/src/services/configmanager.cpp)
hyprpdf_test(tst_recentfilesmodel ${CMAKE_SOURCE_DIR}/src/models/recentfilesmodel.cpp)

hyprpdf_test(tst_pdfdoc ${CMAKE_SOURCE_DIR}/src/services/pdfdoc.cpp)
target_link_libraries(tst_pdfdoc PRIVATE PkgConfig::POPPLER_QT6)

hyprpdf_test(tst_documentmodel
    ${CMAKE_SOURCE_DIR}/src/models/documentmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/services/pdfdoc.cpp)
target_link_libraries(tst_documentmodel PRIVATE PkgConfig::POPPLER_QT6)

hyprpdf_test(tst_documentcontroller
    ${CMAKE_SOURCE_DIR}/src/services/documentcontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/models/documentmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/services/pdfdoc.cpp)
target_link_libraries(tst_documentcontroller PRIVATE PkgConfig::POPPLER_QT6)
