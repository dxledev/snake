# CMake generated Testfile for 
# Source directory: /home/dxle/builds/snake
# Build directory: /home/dxle/builds/snake/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test("game" "/home/dxle/builds/snake/build/game_tests")
set_tests_properties("game" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" _BACKTRACE_TRIPLES "/home/dxle/builds/snake/CMakeLists.txt;32;add_test;/home/dxle/builds/snake/CMakeLists.txt;0;")
add_test("theme" "/home/dxle/builds/snake/build/theme_tests")
set_tests_properties("theme" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" _BACKTRACE_TRIPLES "/home/dxle/builds/snake/CMakeLists.txt;32;add_test;/home/dxle/builds/snake/CMakeLists.txt;0;")
add_test("ui" "/home/dxle/builds/snake/build/ui_tests")
set_tests_properties("ui" PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" _BACKTRACE_TRIPLES "/home/dxle/builds/snake/CMakeLists.txt;32;add_test;/home/dxle/builds/snake/CMakeLists.txt;0;")
add_test("install" "bash" "/home/dxle/builds/snake/tests/install_test.sh" "/home/dxle/builds/snake" "/home/dxle/builds/snake/build")
set_tests_properties("install" PROPERTIES  _BACKTRACE_TRIPLES "/home/dxle/builds/snake/CMakeLists.txt;35;add_test;/home/dxle/builds/snake/CMakeLists.txt;0;")
add_test("cli" "bash" "/home/dxle/builds/snake/tests/cli_test.sh" "/home/dxle/builds/snake/build/snake")
set_tests_properties("cli" PROPERTIES  _BACKTRACE_TRIPLES "/home/dxle/builds/snake/CMakeLists.txt;36;add_test;/home/dxle/builds/snake/CMakeLists.txt;0;")
