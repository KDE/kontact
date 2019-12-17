set(KDEPIM_VERSION_NUMBER "5.13.1")
cmake_minimum_required(VERSION 3.5)
project(kontact VERSION ${KDEPIM_VERSION_NUMBER})
set(KDE_APPLICATIONS_VERSION "19.12.1")
set(KF5_MIN_VERSION "5.63.0")
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(CheckFunctionExists)
include(ECMGeneratePriFile)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)


# Do NOT add quote
set(KDEPIM_DEV_VERSION)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
    set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION} (${KDE_APPLICATIONS_VERSION})")

set(KPIMTEXTEDIT_LIB_VERSION "5.13.0")
set(AKONADI_VERSION "5.13.0")
set(KONTACTINTERFACE_LIB_VERSION "5.13.0")

set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")


set(QT_REQUIRED_VERSION "5.11.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Widgets WebEngine WebEngineWidgets)
set(LIBKDEPIM_LIB_VERSION_LIB "5.13.0")
set(LIBGRANTLEETHEME_LIB_VERSION_LIB "5.13.0")
set(KDEPIM_APPS_LIB_VERSION_LIB "5.13.0")
# Find KF5 package
find_package(KF5DBusAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KCMUtils ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_MIN_VERSION} REQUIRED)


# Find KdepimLibs Package
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5KdepimDBusInterfaces ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)


include_directories(${kontact_SOURCE_DIR} ${kontact_BINARY_DIR})
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
endif()

add_definitions(-DQT_NO_FOREACH)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-DTRANSLATION_DOMAIN=\"kontact\")

add_subdirectory(src)
add_subdirectory(kconf_update)

install(FILES kontact.renamecategories kontact.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
add_subdirectory(doc)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
