# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/game_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/game_tests_autogen.dir/ParseCache.txt"
  "CMakeFiles/snake_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/snake_autogen.dir/ParseCache.txt"
  "CMakeFiles/snake_core_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/snake_core_autogen.dir/ParseCache.txt"
  "CMakeFiles/snake_ui_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/snake_ui_autogen.dir/ParseCache.txt"
  "CMakeFiles/theme_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/theme_tests_autogen.dir/ParseCache.txt"
  "CMakeFiles/ui_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ui_tests_autogen.dir/ParseCache.txt"
  "game_tests_autogen"
  "snake_autogen"
  "snake_core_autogen"
  "snake_ui_autogen"
  "theme_tests_autogen"
  "ui_tests_autogen"
  )
endif()
