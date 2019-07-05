#! /bin/sh
$EXTRACT_GRANTLEE_TEMPLATE_STRINGS `find -name \*.html` >> rc.cpp
$EXTRACTRC src/*.kcfg >> rc.cpp || exit 11
$EXTRACTRC $(find . -name "*.ui" -o -name "*.rc") >> rc.cpp || exit 12
$XGETTEXT rc.cpp src/*.cpp -o $podir/kontact.pot
rm -f rc.cpp ./grantlee-extractor-pot-scripts/grantlee_strings_extractor.pyc
