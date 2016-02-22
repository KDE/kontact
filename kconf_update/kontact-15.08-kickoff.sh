#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/Kontact.desktop/\/org.kde.kontact.desktop/" $kickoffrcname
fi
