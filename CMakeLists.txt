# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(PIM_VERSION "5.21.1")

project(KIMAP VERSION ${PIM_VERSION})


# ECM setup
set(KF5_MIN_VERSION "5.95.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
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

set(KIMAP_LIB_VERSION ${PIM_VERSION})
set(KMIME_LIBS_VERSION "5.21.0")
ecm_setup_version(PROJECT VARIABLE_PREFIX KIMAP
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kimap_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfigVersion.cmake"
                        SOVERSION 5
)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")
########### Find packages ###########
find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES TYPE REQUIRED)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIBS_VERSION} CONFIG REQUIRED)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IMAP")
add_definitions(-DTRANSLATION_DOMAIN=\"libkimap5\")

ecm_set_disabled_deprecation_versions(QT 5.15.0
    KF 5.96.0
)
add_definitions(-DQT_NO_EMIT)


configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5IMAPConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

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
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5IMAPTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5IMAPTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kimap_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIMAP/ COMPONENT Devel
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Imap_QCH
        FILE KF5ImapQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5ImapQchTargets.cmake\")")
endif()

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
