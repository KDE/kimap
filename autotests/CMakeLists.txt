kde_enable_exceptions()

include(ECMMarkAsTest)

find_package(Qt5Test CONFIG REQUIRED)

add_subdirectory(kimaptest)

remove_definitions(-DQT_USE_QSTRINGBUILDER)
macro(KIMAP_UNIT_TESTS)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(${_testname} ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} KF5Imap Qt5::Test kimaptest Qt5::Network KF5::KDELibs4Support)
    set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  endforeach()
endmacro()

########### automated tests ###############

KIMAP_UNIT_TESTS(
  fakeservertest
  testrfccodecs
  testsession
  # loginjobtest FIXME: test hangs forever
  logoutjobtest
  capabilitiesjobtest
  selectjobtest
  createjobtest
  deletejobtest
  fetchjobtest
  renamejobtest
  subscribejobtest
  unsubscribejobtest
  listjobtest
  storejobtest
  imapsettest
  idlejobtest
  quotarootjobtest
  searchjobtest
  getmetadatajobtest
  streamparsertest
  setmetadatajobtest
  appendjobtest
)
