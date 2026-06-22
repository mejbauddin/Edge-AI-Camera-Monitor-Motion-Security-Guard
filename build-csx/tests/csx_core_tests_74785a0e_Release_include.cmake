if(EXISTS "D:/Edge AI Camera Monitor & Motion Security Guard/build-csx/tests/Release/csx_core_tests.exe")
  discover_tests(
    COMMAND "C:/Program Files/CMake/bin/cmake.exe"
      -D [[TEST_EXECUTABLE=D:/Edge AI Camera Monitor & Motion Security Guard/build-csx/tests/Release/csx_core_tests.exe]]
      -D [[TEST_EXECUTOR=]]
    DISCOVERY_ARGS
      -D [[TEST_FILTER=]]
      -D [[TEST_DISCOVERY_EXTRA_ARGS=]]
      -D [[NO_PRETTY_TYPES=FALSE]]
      -D [[NO_PRETTY_VALUES=FALSE]]
      -P [[C:/Program Files/CMake/share/cmake-4.4/Modules/GoogleTest/DiscoverTests.cmake]]
    DISCOVERY_MATCH
      "-- ([^#]+)#([^#]+)#DISABLED=([^#]+)#LOCATION=([^#]*)#"
    DISCOVERY_PROPERTIES
      TIMEOUT [[5]]
      WORKING_DIRECTORY [[D:/Edge AI Camera Monitor & Motion Security Guard/build-csx/tests]]
    TEST_NAME [[\1]]
    TEST_ARGS
      -D [[TEST_FILTER=\2]]
      -D [[TEST_XML_OUTPUT=]]
      -D [[TEST_EXTRA_ARGS=]]
      -P [[C:/Program Files/CMake/share/cmake-4.4/Modules/GoogleTest/LaunchTest.cmake]]
    TEST_PROPERTIES
      DISABLED [[\3]]
      DEF_SOURCE_LINE [[\4]]
      SKIP_REGULAR_EXPRESSION "\\[  SKIPPED \\]"
      WORKING_DIRECTORY [[D:/Edge AI Camera Monitor & Motion Security Guard/build-csx/tests]]
      
    TEST_LIST csx_core_tests_TESTS
  )
  list(TRANSFORM csx_core_tests_TESTS REPLACE ";" "\n")
  list(FILTER csx_core_tests_TESTS EXCLUDE REGEX "(\\[|])")
  list(TRANSFORM csx_core_tests_TESTS REPLACE "\n" [[\\;]])
else()
  add_test(csx_core_tests_NOT_BUILT csx_core_tests_NOT_BUILT)
endif()
