set(summaryeventtest_SRCS summaryeventtest.cpp ../summaryeventinfo.cpp)

kde4_add_unit_test(summaryeventtest TESTNAME summaryeventtest ${summaryeventtest_SRCS})

target_link_libraries(
  summaryeventtest
  KF5::CalendarCore
  KF5::CalendarUtils
)
