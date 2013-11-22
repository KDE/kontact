remove_definitions(-DKDE_DEFAULT_DEBUG_AREA=5600) #remove kontact debug area
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5602)

include_directories(
    ${CMAKE_SOURCE_DIR}/libkdepim/
)


if(QGPGME_FOUND)
  if(BUILD_kmail)
    add_subdirectory(kmail)
  endif()

  if(BUILD_kmail AND BUILD_korganizer)
    add_subdirectory(korganizer)
  endif()

  add_subdirectory(summary)
  add_subdirectory(specialdates)
#AKONADI_PORT: planner
#  add_subdirectory(planner)
endif()

if(BUILD_knotes AND KDEPIMLIBS_KRESOURCES_LIBS)
  add_subdirectory(knotes)
  #add_subdirectory(knotes2)
endif()

if(BUILD_knode)
  add_subdirectory(knode)
endif()

if(BUILD_kjots)
  add_subdirectory(kjots)
endif()

if(BUILD_akregator)
  add_subdirectory(akregator)
endif()

#if(BUILD_ktimetracker)
#  add_subdirectory(ktimetracker)
#endif()

if(BUILD_kaddressbook)
  add_subdirectory(kaddressbook)
endif()

