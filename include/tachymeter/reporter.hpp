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

#ifndef TACHYMETER_REPORTER_HPP
#define TACHYMETER_REPORTER_HPP

#include "measurement.hpp"

#include <vector>
#include <string>

namespace tachymeter {

using result_sequence = std::vector<measurement>;

class reporter {
public:
  virtual ~reporter() {}
  virtual void report(result_sequence const& results, std::string const & name) = 0;
};

}
#endif //TACHYMETER_REPORTER_HPP
