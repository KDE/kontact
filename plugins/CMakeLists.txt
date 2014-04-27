remove_definitions(-DKDE_DEFAULT_DEBUG_AREA=5600) #remove kontact debug area
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5602)

include_directories(
    ${CMAKE_SOURCE_DIR}/libkdepim/
)


add_subdirectory(kmail)
add_subdirectory(korganizer)

add_subdirectory(summary)
add_subdirectory(specialdates)

add_subdirectory(knotes)

add_subdirectory(kjots)

add_subdirectory(akregator)

add_subdirectory(kaddressbook)

