find_package(Qt6 REQUIRED COMPONENTS Test)

function(qkw_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE qkw::qkeyboardwidget Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

qkw_add_test(tst_keyboardlayout tst_keyboardlayout.cpp)
qkw_add_test(tst_keyboardcontroller tst_keyboardcontroller.cpp)
