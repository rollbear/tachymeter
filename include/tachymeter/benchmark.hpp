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

#ifndef TACHYMETER_BENCHMARK_HPP
#define TACHYMETER_BENCHMARK_HPP

#include "reporter.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <iostream>

namespace tachymeter
{

template <typename C>
class benchmark
{
public:
  benchmark(reporter &r_) : r(r_) { }
  void run(int argc, char *argv[], std::ostream &ostr = std::cout);
  template <typename Setup, typename Seq>
  void measure(Seq &&seq, std::string name, typename C::duration min_time);
private:
  class job
  {
  public:
    job(std::string&& n) : job_name(std::move(n)) {}
    virtual ~job() = default;
    virtual void run(reporter &r) = 0;
    bool matches(int argc, char *argv[]) const;
    std::string const& name() const { return job_name;}
  private:
    std::string job_name;
  };
  template <typename Setup, typename Seq>
  class job_t : public job
  {
  public:
    template <typename S,
              typename = std::enable_if_t<std::is_same<Seq,
                                                       std::decay_t<S>>::value>>
    job_t(std::string a_name, S &&a_seq, typename C::duration min_time_)
        : job(std::move(a_name))
        , seq(std::forward<S>(a_seq))
        , min_time(min_time_) { }
    virtual void run(reporter &r) override;
  private:
    Seq                        seq;
    typename C::duration const min_time;
  };
  reporter                          &r;
  std::vector<std::unique_ptr<job>> jobs;
};


template <typename C>
void benchmark<C>::run(int argc, char *argv[], std::ostream &ostr)
{
  if (argc > 1 && argv[1][0] == '-')
  {
    switch (argv[1][1])
    {
      case 'l': for (auto& j : jobs) { ostr << j->name() << '\n';} return;
      default: ostr << "Usage: " << argv[0] << " {-l | <names>}\n"; return;
    }
  }
  for (auto &j : jobs)
  {
    if (j->matches(argc, argv))
    {
      j->run(r);
    }
  }
}

template <typename C>
bool benchmark<C>::job::matches(int argc, char *argv[]) const
{
  return argc == 1 || std::find(argv + 1, argv + argc, job_name) != argv + argc;
}

template <typename C>
template <typename Setup, typename Seq>
void benchmark<C>::measure(Seq &&seq,
                           std::string name,
                           typename C::duration min_time)
{
  using job_type = job_t<Setup, std::decay_t<Seq>>;
  jobs.emplace_back(new job_type(std::move(name),
                                 std::forward<Seq>(seq),
                                 min_time));
}

namespace
{

template <typename T>
bool is_even(T t) { return (t & 1) == 0; }

}
template <typename C>
template <typename Setup, typename Seq>
void benchmark<C>::job_t<Setup, Seq>::run(reporter &r)
{
  using namespace std::chrono_literals;

  result_sequence results;

  for (auto size : seq)
  {
    std::vector<typename C::duration> measured_durations;
    typename C::duration              total_duration{ };
    while (total_duration < min_time
        || measured_durations.size() < 8
        || is_even(measured_durations.size()))
    {
      Setup setup(size);

      auto const before = C::now();
      setup(size);
      auto const after = C::now();

      auto const run_duration = after - before;
      total_duration += run_duration;
      measured_durations.push_back(run_duration);
    }
    std::sort(measured_durations.begin(), measured_durations.end());
    auto const     num_runs = measured_durations.size();
    auto const     lo_q_idx = num_runs / 4;
    auto const     hi_q_idx = num_runs * 3 / 4;
    auto const     qbegin   = measured_durations.begin() + lo_q_idx;
    auto const     qend     = measured_durations.begin() + hi_q_idx;
    auto const     sum      = std::accumulate(qbegin, qend, 0ns);
    uint64_t const low_q    = measured_durations[lo_q_idx].count();
    uint64_t const median   = measured_durations[num_runs / 2].count();
    uint64_t const average  = sum.count() / (hi_q_idx - lo_q_idx);
    uint64_t const high_q   = measured_durations[hi_q_idx].count();
    measurement    m{ size, low_q, median, average, high_q, num_runs };
    results.push_back(m);
  }
  r.report(results, job::name());
}

}

#endif //TACHYMETER_BENCHMARK_HPP
