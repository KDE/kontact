########### install files ###############
kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kontact-admin)

install( PROGRAMS kontact-admin.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )

