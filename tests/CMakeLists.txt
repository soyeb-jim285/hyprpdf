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
