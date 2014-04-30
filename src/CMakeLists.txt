# FIXME: Make it build with this flag
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

# Turn exceptions on
kde_enable_exceptions()

set(kimap_SRCS
   acl.cpp
   acljobbase.cpp
   appendjob.cpp
   capabilitiesjob.cpp
   closejob.cpp
   copyjob.cpp
   createjob.cpp
   deleteacljob.cpp
   deletejob.cpp
   expungejob.cpp
   fetchjob.cpp
   getacljob.cpp
   getmetadatajob.cpp
   getquotajob.cpp
   getquotarootjob.cpp
   global.cpp
   idlejob.cpp
   imapset.cpp
   imapstreamparser.cpp
   job.cpp
   listjob.cpp
   listrightsjob.cpp
   loginjob.cpp
   logoutjob.cpp
   metadatajobbase.cpp
   myrightsjob.cpp
   namespacejob.cpp
   quotajobbase.cpp
   renamejob.cpp
   rfccodecs.cpp
   searchjob.cpp
   selectjob.cpp
   session.cpp
   sessionlogger.cpp
   sessionthread.cpp
   setacljob.cpp
   setmetadatajob.cpp
   setquotajob.cpp
   storejob.cpp
   subscribejob.cpp
   unsubscribejob.cpp
)

add_library(KF5Imap ${kimap_SRCS})

generate_export_header(KF5Imap BASE_NAME kimap)

add_library(KF5::Imap ALIAS KF5Imap)

target_include_directories(KF5Imap INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KImap;${Boost_INCLUDE_DIR};${Sasl2_INCLUDE_DIR}>")
target_include_directories(KF5Imap PUBLIC "$<BUILD_INTERFACE:${KImap_SOURCE_DIR}/src;${KImap_BINARY_DIR}/src>")

target_link_libraries(KF5Imap
  KF5::CoreAddons
  KF5::Mime
  ${Sasl2_LIBRARIES}
)

set_target_properties(KF5Imap PROPERTIES
    VERSION ${KIMAP_VERSION_STRING}
    SOVERSION ${KIMAP_SOVERSION}
    EXPORT_NAME Imap
)

ecm_generate_headers(KImap_CamelCase_HEADERS
  HEADER_NAMES
  Acl
  AclJobBase
  AppendJob
  CapabilitiesJob
  CloseJob
  CopyJob
  CreateJob
  DeleteAclJob
  DeleteJob
  ExpungeJob
  FetchJob
  GetAclJob
  GetMetaDataJob
  GetQuotaJob
  GetQuotaRootJob
  IdleJob
  ImapSet
  Job
  ListJob
  ListRightsJob
  LoginJob
  LogoutJob
  MetaDataJobBase
  MyRightsJob
  QuotaJobBase
  RenameJob
  SearchJob
  SelectJob
  Session
  SessionUiProxy
  SetAclJob
  SetMetaDataJob
  SetQuotaJob
  StoreJob
  SubscribeJob
  UnsubscribeJob

  REQUIRED_HEADERS KImap_HEADERS
)

install(TARGETS KF5Imap EXPORT KF5ImapTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kimap_export.h
    ${KImap_HEADERS}
    namespacejob.h rfccodecs.h # FIXME these two headers don't have camelcase headers in KDE4
    ${KImap_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KImap
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KImap LIB_NAME KF5Imap DEPS "CoreAddons Mime" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
