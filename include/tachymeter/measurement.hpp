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

#ifndef TACHYMETER_MEASUREMENT_HPP
#define TACHYMETER_MEASUREMENT_HPP

#include <cstdint>

namespace tachymeter {

struct measurement {
  uint64_t data_size;
  uint64_t lower_quartile;
  uint64_t median;
  uint64_t average;
  uint64_t upper_quartile;
  uint64_t num_runs;
};
}

#endif //TACHYMETER_MEASUREMENT_HPP
