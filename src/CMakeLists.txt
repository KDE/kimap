# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
# Turn exceptions on
kde_enable_exceptions()
add_library(KPim${KF_MAJOR_VERSION}IMAP)
add_library(KPim${KF_MAJOR_VERSION}::IMAP ALIAS KPim${KF_MAJOR_VERSION}IMAP)
target_sources(KPim${KF_MAJOR_VERSION}IMAP PRIVATE
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
   deletejob.h
   getquotarootjob.h
   fetchjob.h
   listrightsjob.h
   sessionlogger_p.h
   unsubscribejob.h
   loginjob.h
   response_p.h
   sessionthread_p.h
   imapset.h
   closejob.h
   getacljob.h
   acl.h
   idjob.h
   capabilitiesjob.h
   quotajobbase_p.h
   enablejob.h
   metadatajobbase_p.h
   copyjob.h
   session_p.h
   storejob.h
   selectjob.h
   idlejob.h
   setquotajob.h
   job.h
   subscribejob.h
   namespacejob.h
   session.h
   metadatajobbase.h
   sessionuiproxy.h
   createjob.h
   movejob.h
   acljobbase_p.h
   expungejob.h
   imapstreamparser.h
   getmetadatajob.h
   rfccodecs.h
   getquotajob.h
   listjob.h
   deleteacljob.h
   quotajobbase.h
   myrightsjob.h
   common.h
   renamejob.h
   statusjob.h
   setacljob.h
   job_p.h
   searchjob.h
   acljobbase.h
   appendjob.h
   setmetadatajob.h
   logoutjob.h
)

ecm_qt_declare_logging_category(KPim${KF_MAJOR_VERSION}IMAP HEADER kimap_debug.h IDENTIFIER KIMAP_LOG CATEGORY_NAME org.kde.pim.kimap
        OLD_CATEGORY_NAMES log_kimap
        DESCRIPTION "kimap (pim lib)" EXPORT KIMAP)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim${KF_MAJOR_VERSION}IMAP PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(KPim${KF_MAJOR_VERSION}IMAP BASE_NAME kimap)



target_include_directories(KPim${KF_MAJOR_VERSION}IMAP INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAP>")
target_include_directories(KPim${KF_MAJOR_VERSION}IMAP PUBLIC "$<BUILD_INTERFACE:${KIMAP_SOURCE_DIR}/src;${KIMAP_BINARY_DIR}/src>")

if (WIN32)
    set(extra_LIBS ws2_32)
endif()

target_link_libraries(KPim${KF_MAJOR_VERSION}IMAP
PUBLIC
  KPim${KF_MAJOR_VERSION}::Mime
  KF${KF_MAJOR_VERSION}::CoreAddons
  Qt::Network
PRIVATE
  KF${KF_MAJOR_VERSION}::KIOCore
  KF${KF_MAJOR_VERSION}::I18n
  Sasl2::Sasl2
  ${extra_LIBS}
)

set_target_properties(KPim${KF_MAJOR_VERSION}IMAP PROPERTIES
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

install(TARGETS KPim${KF_MAJOR_VERSION}IMAP EXPORT KPim${KF_MAJOR_VERSION}IMAPTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kimap_export.h
    ${KIMAP_HEADERS}
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAP/kimap
    COMPONENT Devel
)

install(FILES
    ${KIMAP_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAP/KIMAP
    COMPONENT Devel
)

ecm_qt_install_logging_categories(EXPORT KIMAP FILE kimap.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
ecm_generate_pri_file(BASE_NAME KIMAP LIB_NAME KPim${KF_MAJOR_VERSION}IMAP DEPS "KMime" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAP)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

if (BUILD_QCH)
    ecm_add_qch(
        KPim${KF_MAJOR_VERSION}Imap_QCH
        NAME KImap
        BASE_NAME KPim${KF_MAJOR_VERSION}Imap
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIMAP_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt${QT_MAJOR_VERSION}Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KIMAP_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
