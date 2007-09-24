
include_directories( ${CMAKE_SOURCE_DIR}/kimap )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############

set(testrfccodecs_SRCS testrfccodecs.cpp )


kde4_add_unit_test(testrfccodecs TESTNAME kimap-testrcfcodecs ${testrfccodecs_SRCS})

target_link_libraries(testrfccodecs ${KDE4_KDECORE_LIBS} kimap ${QT_QTTEST_LIBRARY})

########### install files ###############
