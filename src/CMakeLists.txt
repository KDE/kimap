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
   idjob.cpp
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
   sessionuiproxy.cpp
   setacljob.cpp
   setmetadatajob.cpp
   setquotajob.cpp
   statusjob.cpp
   storejob.cpp
   subscribejob.cpp
   unsubscribejob.cpp
)

ecm_qt_declare_logging_category(kimap_SRCS HEADER kimap_debug.h IDENTIFIER KIMAP_LOG CATEGORY_NAME log_kimap)

add_library(KF5IMAP ${kimap_SRCS})

generate_export_header(KF5IMAP BASE_NAME kimap)

add_library(KF5::IMAP ALIAS KF5IMAP)

target_include_directories(KF5IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP;${Sasl2_INCLUDE_DIR}>")
target_include_directories(KF5IMAP PUBLIC "$<BUILD_INTERFACE:${KIMAP_SOURCE_DIR}/src;${KIMAP_BINARY_DIR}/src>")

target_link_libraries(KF5IMAP
PUBLIC
  KF5::Mime
PRIVATE
  KF5::KIOCore
  KF5::Codecs
  KF5::CoreAddons
  KF5::I18n
  ${Sasl2_LIBRARIES}
)

set_target_properties(KF5IMAP PROPERTIES
    VERSION ${KIMAP_VERSION_STRING}
    SOVERSION ${KIMAP_SOVERSION}
    EXPORT_NAME IMAP
)

ecm_generate_headers(KIMAP_CamelCase_HEADERS
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
  IdJob
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
  StatusJob
  StoreJob
  SubscribeJob
  UnsubscribeJob
  PREFIX KIMAP

  REQUIRED_HEADERS KIMAP_HEADERS
)

install(TARGETS KF5IMAP EXPORT KF5IMAPTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kimap_export.h
    ${KIMAP_HEADERS}
    namespacejob.h rfccodecs.h # FIXME these two headers don't have camelcase headers in KDE4
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP/kimap
    COMPONENT Devel
)

install(FILES
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP/KIMAP
    COMPONENT Devel
)


ecm_generate_pri_file(BASE_NAME KIMAP LIB_NAME KF5IMAP DEPS "Mime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
