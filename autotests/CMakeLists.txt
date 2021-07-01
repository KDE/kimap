# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
kde_enable_exceptions()

include(ECMMarkAsTest)

add_subdirectory(kimaptest)

macro(KIMAP_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} KF5IMAP Qt::Test kimaptest Qt::Network)
    set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
    set_tests_properties(${_testname} PROPERTIES RUN_SERIAL TRUE)
  endforeach()
endmacro()

########### automated tests ###############

KIMAP_UNIT_TESTS(
  fakeservertest
  testrfccodecs
  testsession
  loginjobtest
  logoutjobtest
  capabilitiesjobtest
  closejobtest
  selectjobtest
  createjobtest
  deletejobtest
  enablejobtest
  expungejobtest
  fetchjobtest
  renamejobtest
  subscribejobtest
  unsubscribejobtest
  listjobtest
  storejobtest
  imapsettest
  idjobtest
  idlejobtest
  quotarootjobtest
  searchjobtest
  getmetadatajobtest
  streamparsertest
  setmetadatajobtest
  appendjobtest
  statusjobtest
  movejobtest
)
