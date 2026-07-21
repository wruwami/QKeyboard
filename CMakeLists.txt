cmake_minimum_required(VERSION 3.21)
project(QKeyboardWidget VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(QKW_ENABLE_QML "Build the Qt Quick (QML) view and register QML types" ON)
option(QKW_ENABLE_COVERAGE "Enable gcov/lcov coverage instrumentation (GCC/Clang only)" OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

set(QKW_SOURCES
    src/key_definition.cpp
    src/keyboard_layout.cpp
    src/keyboard_controller.cpp
    src/keyboard_theme.cpp
    src/key_button.cpp
    src/keyboard_widget.cpp
)

set(QKW_HEADERS
    include/qkeyboardwidget/key_definition.h
    include/qkeyboardwidget/keyboard_layout.h
    include/qkeyboardwidget/keyboard_controller.h
    include/qkeyboardwidget/keyboard_theme.h
    include/qkeyboardwidget/key_button.h
    include/qkeyboardwidget/keyboard_widget.h
)

add_library(qkeyboardwidget STATIC ${QKW_SOURCES} ${QKW_HEADERS} resources/qkeyboardwidget.qrc)
add_library(qkw::qkeyboardwidget ALIAS qkeyboardwidget)

target_include_directories(qkeyboardwidget PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(qkeyboardwidget PUBLIC Qt6::Core Qt6::Widgets)

if(QKW_ENABLE_QML)
    find_package(Qt6 REQUIRED COMPONENTS Qml Quick)
    target_compile_definitions(qkeyboardwidget PUBLIC QKW_ENABLE_QML)
    target_link_libraries(qkeyboardwidget PUBLIC Qt6::Qml Qt6::Quick)

    qt6_add_qml_module(qkeyboardwidget
        URI QKeyboardWidget
        VERSION 1.0
        QML_FILES
            qml/KeyboardKey.qml
            qml/KeyboardPanel.qml
    )
endif()

if(QKW_ENABLE_COVERAGE)
    target_compile_options(qkeyboardwidget PUBLIC --coverage -O0 -g)
    target_link_options(qkeyboardwidget PUBLIC --coverage)
endif()

include(GNUInstallDirs)
install(TARGETS qkeyboardwidget
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/qkeyboardwidget DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()
