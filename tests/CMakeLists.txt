# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
kde_enable_exceptions()

add_executable(testimapidle testimapidle.cpp)
target_link_libraries(testimapidle KPim6IMAP Qt::Test KF6::KIOCore Qt::Network)

add_executable(testimapserver testimapserver.cpp)
target_link_libraries(testimapserver KPim6IMAP Qt::Test KF6::KIOCore Qt::Network)
