
include_directories( ${CMAKE_SOURCE_DIR}/kimap )

########### next target ###############

set(testrfccodecs_SRCS testrfccodecs.cpp )

kde4_automoc(${testrfccodecs_SRCS})

kde4_add_unit_test(testrfccodecs TESTNAME kimap-testrcfcodecs ${testrfccodecs_SRCS})

target_link_libraries(testrfccodecs ${KDE4_KDECORE_LIBS} kimap ${QT_QTTEST_LIBRARY})

########### install files ###############
