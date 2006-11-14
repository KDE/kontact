
message( STATUS "Newsticker and weather Kontact plugins disabled." )
add_subdirectory( kaddressbook ) 
if(GPGME_FOUND)
add_subdirectory( kmail ) 
endif(GPGME_FOUND)
add_subdirectory( knotes ) 
add_subdirectory( korganizer ) 
add_subdirectory( summary ) 
#add_subdirectory( weather ) 
add_subdirectory( knode ) 
#add_subdirectory( newsticker ) 
add_subdirectory( specialdates ) 
add_subdirectory( akregator ) 
add_subdirectory( karm ) 


