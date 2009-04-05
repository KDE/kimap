
include_directories( ${CMAKE_SOURCE_DIR}/kimap )
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
    target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kimap kmime)
  ENDFOREACH(_testname)
ENDMACRO(KIMAP_EXECUTABLE_TESTS)


########### automated tests ###############

KIMAP_UNIT_TESTS(
  testrfccodecs
  testsession
)

###########   manual tests  ###############

KIMAP_EXECUTABLE_TESTS(
  testimapserver
)
