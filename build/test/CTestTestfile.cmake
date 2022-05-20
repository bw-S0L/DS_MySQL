# CMake generated Testfile for 
# Source directory: C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test
# Build directory: C:/Users/86198/Desktop/DS/PJ/pj2/myjql/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(test_str "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/build/test/Debug/test_str.exe")
  set_tests_properties(test_str PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;22;add_test;C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(test_str "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/build/test/Release/test_str.exe")
  set_tests_properties(test_str PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;22;add_test;C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(test_str "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/build/test/MinSizeRel/test_str.exe")
  set_tests_properties(test_str PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;22;add_test;C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(test_str "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/build/test/RelWithDebInfo/test_str.exe")
  set_tests_properties(test_str PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;22;add_test;C:/Users/86198/Desktop/DS/PJ/pj2/myjql/test/CMakeLists.txt;0;")
else()
  add_test(test_str NOT_AVAILABLE)
endif()
