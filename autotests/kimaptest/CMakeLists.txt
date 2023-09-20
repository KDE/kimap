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
add_library(KPim6::kimaptest ALIAS kimaptest)
target_link_libraries(kimaptest KPim6IMAP Qt::Test Qt::Network Qt::Core KF6::CoreAddons KF6::KIOCore KF6::I18n)

