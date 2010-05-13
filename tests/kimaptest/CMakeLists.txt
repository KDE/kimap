project(kimaptest)

if(KDE4_BUILD_TESTS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kimaptest_LIB_SRCS
   fakeserver.cpp
)

kde4_add_library(kimaptest STATIC ${kimaptest_LIB_SRCS})
target_link_libraries(kimaptest ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY})
install(TARGETS kimaptest ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kimaptest COMPONENT Devel)

endif(KDE4_BUILD_TESTS)

