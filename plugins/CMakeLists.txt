kde4_header()

add_subdirectory( kaddressbook ) 
add_subdirectory( kmail ) 
add_subdirectory( knotes ) 
add_subdirectory( korganizer ) 
add_subdirectory( summary ) 
add_subdirectory( weather ) 
add_subdirectory( knode ) 
add_subdirectory( newsticker ) 
add_subdirectory( multisynk ) 
add_subdirectory( specialdates ) 
add_subdirectory( akregator ) 
add_subdirectory( karm ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kaddressbook kmail knotes korganizer \
#	summary weather knode newsticker multisynk specialdates akregator karm
