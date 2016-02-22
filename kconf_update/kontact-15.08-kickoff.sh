#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/Kontact.desktop/\/org.kde.kontact.desktop/" $kickoffrcname
fi
