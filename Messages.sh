#! /bin/sh
$EXTRACTRC src/*.kcfg >> rc.cpp || exit 11
$EXTRACTRC $(find . -name "*.ui" -o -name "*.rc") >> rc.cpp || exit 12
$XGETTEXT rc.cpp src/*.cpp plugins/*/*.cpp plugins/*/*.h -o $podir/kontact.pot
