# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

kde_enable_exceptions()

set(kimaptest_SRCS
   fakeserver.cpp
   mockjob.cpp
   sslserver.cpp
)

add_library(kimaptest6 STATIC ${kimaptest_SRCS})
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kimaptest6 PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(kimaptest6)
add_library(KPim6::kimaptest ALIAS kimaptest6)
target_include_directories(KPim6IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/KIMAPTest>")
target_link_libraries(kimaptest6 KPim6IMAP Qt::Test Qt::Network Qt::Core KF6::CoreAddons KF6::KIOCore KF6::I18n)

install(TARGETS kimaptest6 EXPORT KPim6IMAPTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/KIMAPTest/kimaptest COMPONENT Devel)
