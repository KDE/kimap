kde_enable_exceptions()

remove_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_CAST_FROM_BYTEARRAY)

find_package(Qt5Test CONFIG REQUIRED) #for qsignalspy.h

add_executable(testimapidle testimapidle.cpp)
target_link_libraries(testimapidle KF5Imap Qt5::Test)

add_executable(testimapserver testimapserver.cpp)
target_link_libraries(testimapserver KF5Imap Qt5::Test)
