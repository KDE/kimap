project(kimaptest)

if(KDE4_BUILD_TESTS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(${CMAKE_BINARY_DIR}/kimap)

set(kimaptest_LIB_SRCS
   fakeserver.cpp
   mockjob.cpp
)

add_library(kimaptest STATIC ${kimaptest_LIB_SRCS})
generate_export_header(kimaptest)
target_link_libraries(kimaptest ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} ${QT_QTTEST_LIBRARY} kimap)
install(TARGETS kimaptest ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kimaptest COMPONENT Devel)

endif()

