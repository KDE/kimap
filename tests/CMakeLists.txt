# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
kde_enable_exceptions()

add_executable(testimapidle testimapidle.cpp)
target_link_libraries(testimapidle KF5IMAP Qt::Test KF5::KIOCore Qt::Network)

add_executable(testimapserver testimapserver.cpp)
target_link_libraries(testimapserver KF5IMAP Qt::Test KF5::KIOCore Qt::Network)
