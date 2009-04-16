
include_directories( ${CMAKE_SOURCE_DIR}/kimap ${Boost_INCLUDE_DIR})
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO(KIMAP_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} TESTNAME kimap-${_testname} NOGUI ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kimap kmime)
  ENDFOREACH(_testname)
ENDMACRO(KIMAP_UNIT_TESTS)

MACRO(KIMAP_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} kimap kmime)
  ENDFOREACH(_testname)
ENDMACRO(KIMAP_EXECUTABLE_TESTS)

### convenience macro
MACRO(ADD_IMAPLIB_TEST _source)
  set(_test ${_source} fakeserver.cpp )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME kimap-${_name} ${_test})
  target_link_libraries(${_name} kimap ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} )
ENDMACRO(ADD_IMAPLIB_TEST)

### tests
ADD_IMAPLIB_TEST(loginjobtest.cpp)
ADD_IMAPLIB_TEST(logoutjobtest.cpp)
ADD_IMAPLIB_TEST(capabilitiesjobtest.cpp)
ADD_IMAPLIB_TEST(selectjobtest.cpp)
ADD_IMAPLIB_TEST(createjobtest.cpp)
ADD_IMAPLIB_TEST(deletejobtest.cpp)
ADD_IMAPLIB_TEST(renamejobtest.cpp)
ADD_IMAPLIB_TEST(subscribejobtest.cpp)
ADD_IMAPLIB_TEST(unsubscribejobtest.cpp)
ADD_IMAPLIB_TEST(listjobtest.cpp)


########### automated tests ###############

KIMAP_UNIT_TESTS(
  testrfccodecs
  testsession
)

###########   manual tests  ###############

KIMAP_EXECUTABLE_TESTS(
  testimapserver
)
