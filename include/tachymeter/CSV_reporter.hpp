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

#ifndef TACHYMETER_CSV_REPORTER_HPP
#define TACHYMETER_CSV_REPORTER_HPP

#include <iostream>
#include <fstream>

#include "reporter.hpp"

namespace tachymeter
{

class CSV_reporter : public reporter
{
public:
  CSV_reporter(const char* dir, std::ostream *gossip_stream = nullptr)
      : os{ gossip_stream }
      , out_dir{dir}
  { }
  virtual ~CSV_reporter() = default;
  void report(result_sequence const &results, std::string const &name) override;
private:
  std::ostream *os;
  const char* out_dir;
};

inline
void
CSV_reporter::report(tachymeter::result_sequence const &results,
                     std::string const &name)
{
  using namespace std::literals::string_literals;

  std::ofstream out;
  if (out_dir) out.open(out_dir + "/"s + name);

  if (os) *os << "# " << name << "\n#size,lo_q,median,agerage,hi_q,runs\n";

  out << "# " << name << "\n#size,lo_q,median,agerage,hi_q,runs\n";
  for (auto const &m : results)
  {
    if (os)
    {
      *os << m.data_size << ',' << m.lower_quartile << ',' << m.median << ','
          << m.average << ',' << m.upper_quartile << ',' << m.num_runs << '\n';
    }
    out << m.data_size << ',' << m.lower_quartile << ',' << m.median << ','
        << m.average << ',' << m.upper_quartile << ',' << m.num_runs << '\n';
  }
}


}
#endif //TACHYMETER_CSV_REPORTER_HPP
