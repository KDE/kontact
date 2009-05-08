set(summaryeventtest_SRCS summaryeventtest.cpp ../summaryeventinfo.cpp )
kde4_add_unit_test(summaryeventtest TESTNAME summaryeventtest ${summaryeventtest_SRCS})

target_link_libraries(
  summaryeventtest
  kdepim
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
)
