# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
# Turn exceptions on
kde_enable_exceptions()
add_library(KF5IMAP)
add_library(KF5::IMAP ALIAS KF5IMAP)
target_sources(KF5IMAP PRIVATE
   acl.cpp
   acljobbase.cpp
   appendjob.cpp
   capabilitiesjob.cpp
   closejob.cpp
   copyjob.cpp
   createjob.cpp
   deleteacljob.cpp
   deletejob.cpp
   enablejob.cpp
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
   movejob.cpp
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

ecm_qt_declare_logging_category(KF5IMAP HEADER kimap_debug.h IDENTIFIER KIMAP_LOG CATEGORY_NAME org.kde.pim.kimap
        OLD_CATEGORY_NAMES log_kimap
        DESCRIPTION "kimap (pim lib)" EXPORT KIMAP)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5IMAP PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KF5IMAP BASE_NAME kimap)



target_include_directories(KF5IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP>")
target_include_directories(KF5IMAP PUBLIC "$<BUILD_INTERFACE:${KIMAP_SOURCE_DIR}/src;${KIMAP_BINARY_DIR}/src>")

if (WIN32)
    set(extra_LIBS ws2_32)
endif()

target_link_libraries(KF5IMAP
PUBLIC
  KF5::Mime
  KF5::CoreAddons
  Qt::Network
PRIVATE
  KF5::KIOCore
  KF5::Codecs
  KF5::I18n
  Sasl2::Sasl2
  ${extra_LIBS}
)

set_target_properties(KF5IMAP PROPERTIES
    VERSION ${KIMAP_VERSION}
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
  EnableJob
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
  MoveJob
  MyRightsJob
  NamespaceJob
  QuotaJobBase
  RenameJob
  RFCCodecs
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
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP/kimap
    COMPONENT Devel
)

install(FILES
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP/KIMAP
    COMPONENT Devel
)

ecm_qt_install_logging_categories(EXPORT KIMAP FILE kimap.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
ecm_generate_pri_file(BASE_NAME KIMAP LIB_NAME KF5IMAP DEPS "KMime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KIMAP)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
