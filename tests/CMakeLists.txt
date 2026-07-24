find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

function(qkw_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE qkw::qkeyboardwidget Qt${QT_VERSION_MAJOR}::Test)
    # Qt::Widgets pulls in a Windows entry-point helper (Qt5::WinMain /
    # Qt6::EntryPointPrivate) that defaults linked executables to the
    # WIN32 GUI subsystem, which has no console attached — on Windows this
    # silently discards every test binary's own stdout (PASS/FAIL lines),
    # leaving only ctest's own pass/fail summary. Force the console
    # subsystem so test output is actually visible.
    set_target_properties(${name} PROPERTIES WIN32_EXECUTABLE FALSE)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

qkw_add_test(tst_keyboardlayout tst_keyboardlayout.cpp)
qkw_add_test(tst_keyboardcontroller tst_keyboardcontroller.cpp)
qkw_add_test(tst_hangulcomposer tst_hangulcomposer.cpp)
qkw_add_test(tst_kanacomposer tst_kanacomposer.cpp)
qkw_add_test(tst_abstractcomposer tst_abstractcomposer.cpp)
qkw_add_test(tst_keybutton tst_keybutton.cpp)
qkw_add_test(tst_keyboardwidget tst_keyboardwidget.cpp)
qkw_add_test(tst_keyboardtheme tst_keyboardtheme.cpp)

qkw_add_test(tst_translations tst_translations.cpp)
# Reads resources/i18n/*.ts directly from the source tree (they are plain
# Qt Linguist source files, not embedded in the .qrc/compiled to .qm at
# test-run time), the same way tst_quick.cpp locates its QML sources below.
target_compile_definitions(tst_translations PRIVATE QKW_I18N_DIR="${CMAKE_SOURCE_DIR}/resources/i18n")

if(QKW_ENABLE_QML)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickTest)

    add_executable(tst_quick tst_quick.cpp)
    target_link_libraries(tst_quick PRIVATE qkw::qkeyboardwidget Qt${QT_VERSION_MAJOR}::QuickTest)
    # See qkw_add_test()'s WIN32_EXECUTABLE comment above — same reasoning.
    set_target_properties(tst_quick PROPERTIES WIN32_EXECUTABLE FALSE)
    target_compile_definitions(tst_quick PRIVATE
        QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
        QKW_QML_IMPORT_PATH="${CMAKE_SOURCE_DIR}/qml"
    )
    add_test(NAME tst_quick COMMAND tst_quick)
    set_tests_properties(tst_quick PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
