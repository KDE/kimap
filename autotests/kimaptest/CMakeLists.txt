kde_enable_exceptions()

include_directories(${KImap_BINARY_DIR}/src)

set(kimaptest_SRCS
   fakeserver.cpp
   mockjob.cpp
   sslserver.cpp
)

add_library(kimaptest STATIC ${kimaptest_SRCS})
generate_export_header(kimaptest)
target_link_libraries(kimaptest KF5Imap Qt5::Test)

install(TARGETS kimaptest ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kimaptest COMPONENT Devel)