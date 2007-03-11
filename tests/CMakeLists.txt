
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kimap )

########### next target ###############

set(testrfccodecs_SRCS testrfccodecs.cpp )

kde4_automoc(${testrfccodecs_SRCS})

kde4_add_executable(testrfccodecs NOGUI RUN_UNINSTALLED ${testrfccodecs_SRCS})

target_link_libraries(testrfccodecs ${KDE4_KDECORE_LIBS} kimap ${QT_TEST_LIBRARY})

add_test(kimap-testrfccodecs ${EXECUTABLE_OUTPUT_PATH}/testrfccodecs)

########### install files ###############

endif(KDE4_BUILD_TESTS)
