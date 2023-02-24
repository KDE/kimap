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
add_library(KPim${KF_MAJOR_VERSION}::kimaptest ALIAS kimaptest)
target_include_directories(KPim${KF_MAJOR_VERSION}IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAPTest>")
target_link_libraries(kimaptest KPim${KF_MAJOR_VERSION}IMAP Qt::Test Qt::Network Qt::Core KF${KF_MAJOR_VERSION}::CoreAddons KF${KF_MAJOR_VERSION}::KIOCore KF${KF_MAJOR_VERSION}::I18n)

install(TARGETS kimaptest EXPORT KPim${KF_MAJOR_VERSION}IMAPTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  fakeserver.h
  mockjob.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAPTest/kimaptest COMPONENT Devel)
