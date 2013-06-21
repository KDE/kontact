#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5600)

add_subdirectory(plugins)
add_subdirectory(src)

