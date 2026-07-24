add_executable(qkw_widgets_example main.cpp)
target_link_libraries(qkw_widgets_example PRIVATE qkw::qkeyboard Qt${QT_VERSION_MAJOR}::Widgets)
