cmake_minimum_required(VERSION 2.8.12)

project(KIMAP)

# ECM setup
set(KF5_VERSION "5.23.0")
find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${KIMAP_SOURCE_DIR}/cmake)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)


set(PIM_VERSION "5.2.90")

set(KIMAP_LIB_VERSION ${PIM_VERSION})
set(KMIME_LIBS_VERSION "5.2.80")
ecm_setup_version(${KIMAP_LIB_VERSION} VARIABLE_PREFIX KIMAP
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kimap_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5I18n ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5Mime ${KMIME_LIBS_VERSION} CONFIG REQUIRED)

find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES
                             DESCRIPTION "The Cyrus-sasl library"
                             URL "http://www.cyrussasl.org"
                             TYPE REQUIRED
)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5IMAP")
add_definitions(-DTRANSLATION_DOMAIN=\"libkimap5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5IMAPConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5IMAPConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

add_subdirectory(cmake)

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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
