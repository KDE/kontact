#!/bin/sh

sed -i "s/\/Kontact.desktop/\/org.kde.kontact.desktop/" `kf5-config --path config --locate kickoffrc`
