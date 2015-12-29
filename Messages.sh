#! /bin/sh
../grantlee-extractor-pot-scripts/extract_strings_ki18n.py `find -name \*.html` >> src/html.cpp
$EXTRACTRC src/*.kcfg >> rc.cpp || exit 11
$EXTRACTRC $(find . -name "*.ui" -o -name "*.rc") >> rc.cpp || exit 12
$XGETTEXT rc.cpp src/*.cpp plugins/*/*.cpp plugins/*/*.h -o $podir/kontact.pot
rm -f rc.cpp src/html.cpp ../grantlee-extractor-pot-scripts/grantlee_strings_extractor.pyc
