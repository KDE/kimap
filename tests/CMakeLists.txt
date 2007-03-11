
if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/..   )

########### next target ###############

set(testkimap_SRCS ${kimap_SRCS} testkimap.cpp )

kde4_automoc(${testkimap_SRCS})

kde4_add_executable(testkimap ${testkimap_SRCS})

target_link_libraries(testkimap  ${KDE4_KDECORE_LIBS} kimap )

add_test(kimap-testkimap ${EXECUTABLE_OUTPUT_PATH}/testkimap)

########### install files ###############

endif(KDE4_BUILD_TESTS)
