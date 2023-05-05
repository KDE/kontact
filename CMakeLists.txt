set(PIM_VERSION "5.240.40")
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(kontact VERSION ${PIM_VERSION})
set(RELEASE_SERVICE_VERSION "23.07.40")
set(KF_MIN_VERSION "5.240.0")
set(QT_REQUIRED_VERSION "6.4.0")
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)


# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
    set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")

set(KPIMTEXTEDIT_LIB_VERSION "5.240.40")
set(AKONADI_VERSION "5.240.40")
set(KONTACTINTERFACE_LIB_VERSION "5.240.40")
set(PIMCOMMON_LIB_VERSION "5.240.40")

set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "6")


find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Widgets WebEngineWidgets)
set(LIBKDEPIM_LIB_VERSION "5.240.40")
set(LIBGRANTLEETHEME_LIB_VERSION "5.240.40")
# Find KF6 package
find_package(KF6I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6DBusAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KCMUtils ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Crash ${KF_MIN_VERSION} REQUIRED)
find_package(KF6IconThemes ${KF_MIN_VERSION} REQUIRED)
find_package(KF6GuiAddons ${KF_MIN_VERSION} REQUIRED)
find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)


# Find KdepimLibs Package
find_package(KPim6TextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KPim6KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6GrantleeTheme ${LIBGRANTLEETHEME_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6PimCommon ${PIMCOMMON_LIB_VERSION} CONFIG REQUIRED)


include_directories(${kontact_SOURCE_DIR} ${kontact_BINARY_DIR})
ecm_set_disabled_deprecation_versions(QT 6.5  KF 5.240.0)


add_definitions(-DTRANSLATION_DOMAIN=\"kontact\")
option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()
add_subdirectory(src)

ecm_qt_install_logging_categories(
        EXPORT KONTACT
        FILE kontact.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
if(KF6DocTools_FOUND)
    if (QT_MAJOR_VERSION STREQUAL "5")
        kdoctools_install(po)
        add_subdirectory(doc)
    else()
        MESSAGE(STATUS "DOC and PO not generated until deprecated element was removed in documentation")
    endif()
endif()
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
