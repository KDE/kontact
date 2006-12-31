
message( STATUS "Newsticker Kontact plugins disabled." )
add_subdirectory( kaddressbook ) 
if(GPGME_FOUND)
add_subdirectory( kmail ) 
add_subdirectory( korganizer )
add_subdirectory( summary )
add_subdirectory( specialdates )
endif(GPGME_FOUND)
add_subdirectory( knotes ) 
add_subdirectory( knode ) 
#add_subdirectory( newsticker ) 
add_subdirectory( akregator ) 
add_subdirectory( karm ) 

set(kweather_file ${DBUS_INTERFACES_DIR}/org.kde.kweather.service.xml)
find_file(KWEATHER_INTERFACE_FILE_FOUND ${kweather_file} )

if(KWEATHER_INTERFACE_FILE_FOUND)
	add_subdirectory( weather )
else(KWEATHER_INTERFACE_FILE_FOUND)
	MESSAGE(STATUS "kweather plugins will not compile. Kdetoys must be install before to compile it")
endif(KWEATHER_INTERFACE_FILE_FOUND)

