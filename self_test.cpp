/*
 * Tachymeter C++ micro benchmark
 *
 * Copyright Björn Fahller 2015
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/rollbear/tachymeter
 */

#include <tachymeter/benchmark.hpp>
#include <tachymeter/CSV_reporter.hpp>
#include <tachymeter/seq.hpp>
#include <trompeloeil.hpp>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

using trompeloeil::_;
using namespace std::chrono_literals;
class test_clock
{
public:
  test_clock() { instance = this;}
  ~test_clock() { instance = nullptr;}
  using duration = std::chrono::milliseconds;
  using time_point = std::chrono::milliseconds;
  static time_point now() { return instance->mock_now(); }
  MAKE_MOCK0(mock_now, time_point());
private:
  static test_clock* instance;
};

test_clock* test_clock::instance;

class mock_reporter : public tachymeter::reporter
{
public:
  MAKE_MOCK2(report, void(tachymeter::result_sequence const& results, std::string const & name));
};

class test_mock
{
public:
  MAKE_MOCK1(constr, void(std::size_t));
  MAKE_MOCK1(call, void(std::size_t));
};

test_mock* mock_tests[] = { nullptr, nullptr, nullptr };

template <std::size_t idx>
class dummy_test
{
public:
  dummy_test(std::size_t size) { mock_tests[idx]->constr(size);}
  void operator()(std::size_t size) { mock_tests[idx]->call(size);}
};

TEST_CASE("seq object repeats given numbers in range for loop", "[sequences]")
{
  auto s = tachymeter::seq(1, 5, 3, 1, 8, 33);
  unsigned nums[] { 1, 5, 3, 1, 8, 33};
  auto ptr = std::begin(nums);
  for (auto const& n : s)
  {
    REQUIRE(n == *ptr);
    ++ptr;
  }
  REQUIRE(ptr == std::end(nums));
}

TEST_CASE("power object repeats power series until ceil inclusive in range for loop", "[sequences]")
{
  auto s = tachymeter::powers(1, 128, 2);
  unsigned nums[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
  auto ptr = std::begin(nums);
  for (auto const& n : s)
  {
    REQUIRE(n == *ptr);
    ++ptr;
  }
  REQUIRE(ptr == std::end(nums));
}

TEST_CASE("power object with seq repeats seq mutliplied by each power in range for loop", "[sequence]")
{
  auto s = tachymeter::powers(tachymeter::seq(1,2,5), 1, 100, 10);
  unsigned nums[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500 };
  auto ptr = std::begin(nums);
  for (auto const& n : s)
  {
    REQUIRE(n == *ptr);
    ++ptr;
  }
  REQUIRE(ptr == std::end(nums));
}

TEST_CASE("seq object with powers repeats power series multiplied by each num in seq in range for loop", "[sequence]")
{
  auto s = tachymeter::seq(tachymeter::powers(1, 8, 2), 1, 20, 30);
  unsigned nums[] = { 1, 2, 4, 8, 20, 40, 80, 160, 30, 60, 120, 240 };
  auto ptr = std::begin(nums);
  for (auto const& n : s)
  {
    REQUIRE(n == *ptr);
    ++ptr;
  }
  REQUIRE(ptr == std::end(nums));
}

TEST_CASE("benchmark::run runs a test at least 9 times, even if min time is reached on first", "[benchmark]")
{
  mock_reporter reporter;
  ALLOW_CALL(reporter, report(_,_));
  test_clock clock;
  int tick = 0;
  ALLOW_CALL(clock, mock_now())
  .LR_RETURN(std::chrono::milliseconds(tick++));
  tachymeter::benchmark<test_clock> b(reporter);
  b.measure<dummy_test<0>>(tachymeter::seq(123), "apa", 1ms);
  test_mock m;
  mock_tests[0] = &m;
  REQUIRE_CALL(m, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m, call(123U))
  .TIMES(9);
  char* argv[] = { const_cast<char*>("apa")};
  std::ostringstream os;
  b.run(1, argv, os);
  REQUIRE(os.str() == "");
}

TEST_CASE("benchmark::run runs a test as many times as needed to reach min time", "[benchmark]")
{
  mock_reporter reporter;
  ALLOW_CALL(reporter, report(_,_));
  test_clock clock;
  int tick = 0;
  ALLOW_CALL(clock, mock_now())
  .LR_RETURN(std::chrono::milliseconds(tick++));
  tachymeter::benchmark<test_clock> b(reporter);
  b.measure<dummy_test<0>>(tachymeter::seq(123), "apa", 100ms);
  test_mock m;
  mock_tests[0] = &m;
  REQUIRE_CALL(m, constr(123U))
  .TIMES(101);
  REQUIRE_CALL(m, call(123U))
  .TIMES(101);
  char* argv[] = { const_cast<char*>("apa")};
  std::ostringstream os;
  b.run(1, argv, os);
  REQUIRE(os.str() == "");
}

TEST_CASE("benchmark::run runs the tests in the sequence provided", "[benchmark]")
{
  mock_reporter reporter;
  ALLOW_CALL(reporter, report(_,_));
  test_clock clock;
  int tick = 0;
  ALLOW_CALL(clock, mock_now())
  .LR_RETURN(std::chrono::milliseconds(tick++));
  tachymeter::benchmark<test_clock> b(reporter);
  b.measure<dummy_test<0>>(tachymeter::seq(10, 20, 30, 40), "apa", 1ms);
  test_mock m;
  mock_tests[0] = &m;
  trompeloeil::sequence constr_seq;
  trompeloeil::sequence call_seq;

  REQUIRE_CALL(m, constr(10U))
   .TIMES(9)
   .IN_SEQUENCE(constr_seq);

  REQUIRE_CALL(m, call(10U))
   .TIMES(9)
   .IN_SEQUENCE(call_seq);

  REQUIRE_CALL(m, constr(20U))
   .TIMES(9)
   .IN_SEQUENCE(constr_seq);
  REQUIRE_CALL(m, call(20U))
   .TIMES(9)
   .IN_SEQUENCE(call_seq);

  REQUIRE_CALL(m, constr(30U))
   .TIMES(9)
   .IN_SEQUENCE(constr_seq);
  REQUIRE_CALL(m, call(30U))
   .TIMES(9)
   .IN_SEQUENCE(call_seq);

  REQUIRE_CALL(m, constr(40U))
   .TIMES(9)
   .IN_SEQUENCE(constr_seq);
  REQUIRE_CALL(m, call(40U))
   .TIMES(9)
   .IN_SEQUENCE(call_seq);

  char* argv[] = { const_cast<char*>("apa")};
  std::ostringstream os;
  b.run(1, argv, os);
  REQUIRE(os.str() == "");
}

TEST_CASE("benchmark::run runs the tests in the order added and reports them with their name", "[benchmark]")
{
  mock_reporter reporter;
  test_clock clock;
  int tick = 0;
  ALLOW_CALL(clock, mock_now())
  .LR_RETURN(std::chrono::milliseconds(tick++));
  tachymeter::benchmark<test_clock> b(reporter);

  b.measure<dummy_test<0>>(tachymeter::seq(123), "first", 1ms);
  b.measure<dummy_test<1>>(tachymeter::seq(123), "second", 1ms);
  b.measure<dummy_test<2>>(tachymeter::seq(123), "third", 1ms);

  test_mock m1;
  mock_tests[0] = &m1;
  REQUIRE_CALL(m1, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m1, call(123U))
  .TIMES(9);

  test_mock m2;
  mock_tests[1] = &m2;
  REQUIRE_CALL(m2, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m2, call(123U))
  .TIMES(9);

  test_mock m3;
  mock_tests[2] = &m3;
  REQUIRE_CALL(m3, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m3, call(123U))
  .TIMES(9);

  trompeloeil::sequence report_seq;
  REQUIRE_CALL(reporter, report(_, "first"))
  .IN_SEQUENCE(report_seq);
  REQUIRE_CALL(reporter, report(_, "second"))
  .IN_SEQUENCE(report_seq);
  REQUIRE_CALL(reporter, report(_, "third"))
  .IN_SEQUENCE(report_seq);

  char* argv[] = { const_cast<char*>("apa")};
  std::ostringstream os;
  b.run(1, argv, os);
  REQUIRE(os.str() == "");
}

TEST_CASE("benchmark::run runs the tests matching the parameters", "[benchmark]")
{
  mock_reporter                     reporter;
  test_clock                        clock;
  int                               tick = 0;
  ALLOW_CALL(clock, mock_now())
  .LR_RETURN(std::chrono::milliseconds(tick++));
  tachymeter::benchmark<test_clock> b(reporter);

  b.measure<dummy_test<0>>(tachymeter::seq(123), "first", 1ms);
  b.measure<dummy_test<1>>(tachymeter::seq(123), "second", 1ms);
  b.measure<dummy_test<2>>(tachymeter::seq(123), "third", 1ms);

  test_mock m1;
  mock_tests[0] = &m1;
  REQUIRE_CALL(m1, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m1, call(123U))
  .TIMES(9);

  test_mock m2;
  mock_tests[1] = &m2;

  test_mock m3;
  mock_tests[2] = &m3;
  REQUIRE_CALL(m3, constr(123U))
  .TIMES(9);
  REQUIRE_CALL(m3, call(123U))
  .TIMES(9);

  trompeloeil::sequence report_seq;
  REQUIRE_CALL(reporter, report(_, "first"))
  .IN_SEQUENCE(report_seq);
  REQUIRE_CALL(reporter, report(_, "third"))
  .IN_SEQUENCE(report_seq);

  char *argv[] = {
      const_cast<char *>("apa"),
      const_cast<char*>("third"),
      const_cast<char*>("first")
  };
  std::ostringstream os;
  b.run(3, argv, os);
  REQUIRE(os.str() == "");
}

TEST_CASE("benchmark::run with -l flag lists the tests but doesn't run anything", "[benchmark]")
{
  mock_reporter                     reporter;
  test_clock                        clock;

  tachymeter::benchmark<test_clock> b(reporter);

  b.measure<dummy_test<0>>(tachymeter::seq(123), "first", 1ms);
  b.measure<dummy_test<1>>(tachymeter::seq(123), "second", 1ms);
  b.measure<dummy_test<2>>(tachymeter::seq(123), "third", 1ms);

  test_mock m1;
  mock_tests[0] = &m1;

  test_mock m2;
  mock_tests[1] = &m2;

  test_mock m3;
  mock_tests[2] = &m3;

  char *argv[] = {
    const_cast<char *>("apa"),
    const_cast<char*>("-l")
  };
  std::ostringstream os;
  b.run(2, argv, os);

  REQUIRE(os.str() == "first\nsecond\nthird\n");
}

TEST_CASE("benchmark::run with unknown flag gives usage", "[benchmark]")
{
  mock_reporter                     reporter;
  test_clock                        clock;

  tachymeter::benchmark<test_clock> b(reporter);

  b.measure<dummy_test<0>>(tachymeter::seq(123), "first", 1ms);
  b.measure<dummy_test<1>>(tachymeter::seq(123), "second", 1ms);
  b.measure<dummy_test<2>>(tachymeter::seq(123), "third", 1ms);

  test_mock m1;
  mock_tests[0] = &m1;

  test_mock m2;
  mock_tests[1] = &m2;

  test_mock m3;
  mock_tests[2] = &m3;

  char *argv[] = {
      const_cast<char *>("apa"),
      const_cast<char*>("-w")
  };
  std::ostringstream os;
  b.run(2, argv, os);

  REQUIRE(os.str() == "Usage: apa {-l | <names>}\n");
}