# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

kde_enable_exceptions()

set(kimaptest_SRCS
   fakeserver.cpp
   mockjob.cpp
   sslserver.cpp
)

add_library(kimaptest STATIC ${kimaptest_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kimaptest PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(kimaptest)
add_library(KF5::kimaptest ALIAS kimaptest)
target_include_directories(KF5IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KIMAPTest>")
target_link_libraries(kimaptest KF5IMAP Qt::Test Qt::Network Qt::Core KF5::CoreAddons KF5::KIOCore KF5::I18n)

install(TARGETS kimaptest EXPORT KF5IMAPTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIMAPTest/kimaptest COMPONENT Devel)
