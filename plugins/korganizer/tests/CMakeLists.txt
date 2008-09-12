set (summaryeventtest_SRCS summaryeventtest.cpp ../summaryeventinfo.cpp )
kde4_add_unit_test (summaryeventtest TESTNAME summaryeventtest ${summaryeventtest_SRCS})

target_link_libraries ( summaryeventtest
                        ${QT_QTTEST_LIBRARY}
                        ${QT_QTCORE_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        ${KDE4_KCAL_LIBS} )
###${KDE4_KDEUI_LIBS} )