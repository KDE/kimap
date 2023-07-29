# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.23.90")

project(KIMAP VERSION ${PIM_VERSION})


# ECM setup
set(KF_MIN_VERSION "5.105.0")

find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()


set(KIMAP_LIB_VERSION ${PIM_VERSION})
set(KMIME_LIBS_VERSION "5.23.80")
ecm_setup_version(PROJECT VARIABLE_PREFIX KIMAP
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kimap_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IMAPConfigVersion.cmake"
                        SOVERSION 5
)
option(BUILD_WITH_COMPAT_LIBS "Generate compat cmake file (Set at false will allow to co-install lib)" ON)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")
########### Find packages ###########
find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES TYPE REQUIRED)
find_package(KF${KF_MAJOR_VERSION}CoreAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF${KF_MAJOR_VERSION}KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KPim${KF_MAJOR_VERSION}Mime ${KMIME_LIBS_VERSION} CONFIG REQUIRED)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim${KF_MAJOR_VERSION}IMAP")
set(KIMAP_KF5_COMPAT FALSE)
add_definitions(-DTRANSLATION_DOMAIN=\"libkimap5\")

ecm_set_disabled_deprecation_versions(QT 6.4
     KF 5.105.0
)
add_definitions(-DQT_NO_EMIT)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED Test)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

########### Install Files ###########



install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IMAPConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IMAPConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPim${KF_MAJOR_VERSION}IMAPTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPim${KF_MAJOR_VERSION}IMAPTargets.cmake NAMESPACE KPim${KF_MAJOR_VERSION}::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kimap_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim${KF_MAJOR_VERSION}/KIMAP/ COMPONENT Devel
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim${KF_MAJOR_VERSION}Imap_QCH
        FILE KPim${KF_MAJOR_VERSION}ImapQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim${KF_MAJOR_VERSION}ImapQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimIMAPConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IMAPConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

if (BUILD_WITH_COMPAT_LIBS)
if (QT_MAJOR_VERSION STREQUAL "5")
##
# TODO: Backwards compatiblity. Remove in next major version
##
set(CMAKECONFIG_INSTALL_DIR_KF5 "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IMAP")
set(KIMAP_KF5_COMPAT TRUE)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimIMAPConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR_KF5}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    COMPONENT Devel
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}IMAPConfigVersion.cmake"
    RENAME "KF5IMAPConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    COMPONENT Devel
)
install(EXPORT KPim${KF_MAJOR_VERSION}IMAPTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
    FILE KPim${KF_MAJOR_VERSION}IMAPTargets.cmake
    NAMESPACE KF5::
)
if (BUILD_QCH)
    install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KPim${KF_MAJOR_VERSION}ImapQchTargets.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR_KF5}"
        COMPONENT Devel
    )
endif()

endif()
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
