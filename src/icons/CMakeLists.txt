file(GLOB icons "*-apps-kontact.*")
ecm_install_icons(
    ICONS ${icons}
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
