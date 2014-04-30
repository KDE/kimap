kde_enable_exceptions()

set(kimaptest_SRCS
   fakeserver.cpp
   mockjob.cpp
   sslserver.cpp
)

add_library(kimaptest STATIC ${kimaptest_SRCS})
generate_export_header(kimaptest)
target_link_libraries(kimaptest KF5Imap Qt5::Test)

install(TARGETS kimaptest ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/kimaptest COMPONENT Devel)
