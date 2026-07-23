cmake_minimum_required(VERSION 3.21)
project(QKeyboardWidget VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# MSVC assumes the active system code page for narrow string-literal source
# text unless told otherwise, silently mangling the non-ASCII (Hangul)
# literals in hangul_composer.cpp/tst_hangulcomposer.cpp; force UTF-8 source
# and execution character sets so they round-trip correctly on Windows. A
# no-op on GCC/Clang, which already assume UTF-8 source.
if(MSVC)
    add_compile_options(/utf-8)
endif()

option(QKW_ENABLE_QML "Build the Qt Quick (QML) view and register QML types" ON)
option(QKW_ENABLE_COVERAGE "Enable gcov/lcov coverage instrumentation (GCC/Clang only)" OFF)
option(QKW_BUILD_TRANSLATIONS "Compile resources/i18n/*.ts UI-string translations into .qm files" ON)

include(GNUInstallDirs)

# Qt5 (broad 5.x) through the latest Qt6 are all supported: find whichever
# major version is available first, then request components from it.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(QKW_SOURCES
    src/key_definition.cpp
    src/keyboard_layout.cpp
    src/keyboard_controller.cpp
    src/keyboard_theme.cpp
    src/key_button.cpp
    src/keyboard_widget.cpp
    src/hangul_composer.cpp
)

set(QKW_HEADERS
    include/qkeyboardwidget/key_definition.h
    include/qkeyboardwidget/keyboard_layout.h
    include/qkeyboardwidget/keyboard_controller.h
    include/qkeyboardwidget/keyboard_theme.h
    include/qkeyboardwidget/key_button.h
    include/qkeyboardwidget/keyboard_widget.h
    include/qkeyboardwidget/hangul_composer.h
)

# No explicit STATIC/SHARED: follows BUILD_SHARED_LIBS (CMake's own default
# is STATIC when unset), so downstream consumers pick the link type with the
# standard -DBUILD_SHARED_LIBS=ON/OFF rather than this project hardcoding one.
add_library(qkeyboardwidget ${QKW_SOURCES} ${QKW_HEADERS} resources/qkeyboardwidget.qrc)
add_library(qkw::qkeyboardwidget ALIAS qkeyboardwidget)

target_include_directories(qkeyboardwidget PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Generates qkw_export.h with QKW_EXPORT (dllexport/dllimport on Windows,
# default-visible on Linux/macOS, where GCC/Clang's default visibility
# already makes this a no-op). Needed for the SHARED case: without it, a
# Windows DLL build exports no symbols at all and every consumer fails to
# link. GenerateExportHeader also emits a static_define name we apply
# ourselves below, since it can't see BUILD_SHARED_LIBS at generation time —
# only the target's own STATIC/SHARED TYPE, resolved after add_library()
# above.
include(GenerateExportHeader)
generate_export_header(qkeyboardwidget
    BASE_NAME QKW
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/qkeyboardwidget/qkw_export.h"
)
get_target_property(QKW_LIBRARY_TYPE qkeyboardwidget TYPE)
if(QKW_LIBRARY_TYPE STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(qkeyboardwidget PUBLIC QKW_STATIC_DEFINE)
endif()
target_include_directories(qkeyboardwidget PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(qkeyboardwidget PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)

if(QKW_ENABLE_QML)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Qml Quick)
    target_sources(qkeyboardwidget PRIVATE
        src/qml_registration.cpp
        include/qkeyboardwidget/qml_registration.h
    )
    target_compile_definitions(qkeyboardwidget PUBLIC QKW_ENABLE_QML)
    target_link_libraries(qkeyboardwidget PUBLIC Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Quick)

    # qml/QKeyboardWidget/KeyboardKey.qml, qml/QKeyboardWidget/KeyboardPanel.qml,
    # and the hand-written qml/QKeyboardWidget/qmldir are shipped as plain
    # files (not a compiled qt_add_qml_module, which is Qt6-only): the
    # QKeyboardWidget/ subdirectory name is required by Qt's module-import
    # resolution ("import QKeyboardWidget 1.0" looks for
    # <import path>/QKeyboardWidget/qmldir). Host apps add qml/ (the parent
    # of QKeyboardWidget/) to the QML import path and call
    # qkw::registerQmlTypes() before loading QML that imports
    # "QKeyboardWidget". See README.md.
    install(DIRECTORY qml/ DESTINATION ${CMAKE_INSTALL_DATADIR}/qkeyboardwidget/qml)
endif()

if(QKW_ENABLE_COVERAGE)
    target_compile_options(qkeyboardwidget PUBLIC --coverage -O0 -g)
    target_link_options(qkeyboardwidget PUBLIC --coverage)
endif()

if(QKW_BUILD_TRANSLATIONS)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS LinguistTools)

    set(QKW_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/i18n/qkeyboardwidget_ko.ts)

    # English needs no .ts/.qm: QCoreApplication::translate() falls back to
    # the (English) source text whenever no translator is installed for the
    # active locale, so "en" is the implicit default with nothing to compile.
    #
    # CMake command names can't be built by string interpolation directly
    # ("qt${QT_VERSION_MAJOR}_add_translation(...)" is a parse error, not a
    # dynamic call) — cmake_language(CALL ...) is the actual mechanism for
    # invoking a command whose name is only known at configure time.
    cmake_language(CALL "qt${QT_VERSION_MAJOR}_add_translation" QKW_QM_FILES ${QKW_TS_FILES})

    add_custom_target(qkeyboardwidget_translations ALL DEPENDS ${QKW_QM_FILES})
    add_dependencies(qkeyboardwidget qkeyboardwidget_translations)

    install(FILES ${QKW_QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/qkeyboardwidget/i18n)
endif()

install(TARGETS qkeyboardwidget
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/qkeyboardwidget DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/qkeyboardwidget/qkw_export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qkeyboardwidget)

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()
