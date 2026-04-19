cmake_minimum_required(VERSION 3.20)
project(hyprpdf VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core Gui Qml Quick QuickControls2 Pdf Svg Widgets DBus Test
)

set(HYPRPDF_DATA_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH
    "Runtime data dir (themes, qml). Packagers override to /usr/share/hyprpdf")
set(HYPRPDF_SOURCE_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH
    "Build source dir (for QML loadFromModule). Do not override.")

add_compile_definitions(
    HYPRPDF_DATA_DIR="${HYPRPDF_DATA_DIR}"
    HYPRPDF_SOURCE_DIR="${HYPRPDF_SOURCE_DIR}"
    HYPRPDF_VERSION="${PROJECT_VERSION}"
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
