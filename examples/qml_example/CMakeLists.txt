add_executable(qkw_qml_example main.cpp)
target_link_libraries(qkw_qml_example PRIVATE qkw::qkeyboardwidget Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Quick)

# For this from-source example, point the QML import path straight at the
# library's qml/ source directory instead of a deployed install location
# (see CMakeLists.txt's `install(DIRECTORY qml/ ...)` for how a packaged
# app would deploy it instead). Same idea for Main.qml itself: loaded
# straight off disk rather than through a resource, to keep this example
# minimal.
target_compile_definitions(qkw_qml_example PRIVATE
    QKW_QML_IMPORT_PATH="${CMAKE_SOURCE_DIR}/qml"
    QKW_EXAMPLE_MAIN_QML="${CMAKE_CURRENT_SOURCE_DIR}/Main.qml"
)
