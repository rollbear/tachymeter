## *Tachymeter*

> *Tachymeter* watch scale [(Wikipedia)](https://en.wikipedia.org/wiki/Tachymeter_(watch))  
> A tachymeter [ta-kim-i-ter, tuh-] scale is a scale sometimes inscribed around
> the rim of an analog watch. It can be used to compute a speed based on travel
> time or measure distance based on speed...

What is it?
-----------

A header only C++14 micro benchmark.

Although there are many such frameworks available, I found none that fit my
needs. There probably are some that I haven't found.

Measurement setups are classes, the constructor does the setup, the destructor
teardown, and a function call operator does the action to be measured on. Both
the constructor and function call operator accepts the data size as parameter.

Each measurement is run as many times as is required to reach a minimum total
run-time, according to the clock specified.

The results from each measurement is a sequence of test sizes, the measured 
times (lower quartile, median, average, and upper quartile) and the number of
runs required to collect the data.

Every measurement has a name that can be identified in the reports, and used
to filter which measurements to run.

The test sizes follow sequences that can be specified as power series, or an
explicit list of sizes (or combine them both.)

A CSV_reporter class is provided that generates reports in CSV format using
the names of the measurements. You can write your own reporters following the
`reporter` interface.

An example comparing the performance of std::sort<int> and qsort:

```Cpp
#include <tachymeter/seq.hpp>
#include <tachymeter/CSV_reporter.hpp>
#include <tachymeter/benchmark.hpp>

#include <algorithm>
#include <vector>
#include <random>
#include <cassert>

using namespace std::chrono_literals;

class rand_vector
{
public:
  rand_vector(std::size_t s)
  {
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dist(1, 1000000000);
    v.reserve(s);
    while(s--) v.push_back(dist(gen));
  }
protected:
  std::vector<int> v;
};
class sort_measure : public rand_vector
{
public:
  using rand_vector::rand_vector;
  void operator()(std::size_t s)
  {
    assert(s == v.size());
    std::sort(std::begin(v), std::end(v));
  }
};

int cmp(const void* lh, const void *rh)
{
  return *static_cast<const int*>(lh) - *static_cast<const int*>(rh);
}

class qsort_measure : public rand_vector
{
public:
  using rand_vector::rand_vector;
  void operator()(std::size_t s)
  {
    assert(s == v.size());
    qsort(v.data(), s, sizeof(int), cmp);
  }
};

int main(int argc, char *argv[])
{
  auto sizes = tachymeter::powers(tachymeter::seq(1,2,5), 1, 100000, 10);
  tachymeter::CSV_reporter report(nullptr, &std::cout);
  tachymeter::benchmark<std::chrono::steady_clock> b(report);
  b.measure<sort_measure>(sizes, "std::sort", 10ms);
  b.measure<qsort_measure>(sizes, "qsort", 10ms);
  b.run(argc, argv);
}
```

The program measures the performance of std::sort<int> and qsort. The size
series is the power of 10 seq from 1 to 100000, each multiplied with 1, 2,
and 5. The `nullptr` for the `CSV_reporter` constructor is to say no files are
to be saved, just show the findings on `std::cout`. The benchmark uses
`std::chrono::steady_clock`.

Each measurement has a lower bound of at least 10ms each to get reliable
results.

Running the program with "-l" as parameter lists the measurements:

```
std::sort
qsort
```

Running the program without any parameters runs all measurements. As an
alternative, the command line parameters can list the measurements to run.

Running bots tests generated the following on one run:

```
# std::sort
#size,lo_q,median,agerage,hi_q,runs
1,42,42,42,42,234417
2,46,59,56,68,173145
5,97,110,109,120,92117
10,201,217,216,234,45959
20,593,627,627,667,15761
50,1899,1980,1981,2064,5035
100,4446,4577,4579,4718,2181
200,10209,10407,10400,10590,961
500,29799,30151,30163,30544,333
1000,66249,66726,66711,67235,151
2000,144104,145607,145466,147214,69
5000,405205,406747,406865,409766,25
10000,871592,874993,875502,883022,13
20000,1871583,1882594,1879144,1891796,9
50000,5056638,5073650,5071550,5120253,9
100000,10837461,10842526,10841827,10889800,9
200000,22850336,22911924,22888958,23109985,9
500000,61082064,61317761,61336158,61991563,9
# qsort
#size,lo_q,median,agerage,hi_q,runs
1,50,51,51,53,192119
2,76,86,84,93,117393
5,194,208,208,222,48015
10,456,479,479,502,20847
20,1101,1139,1138,1176,8771
50,3526,3594,3595,3667,2757
100,8092,8192,8191,8301,1219
200,18301,18459,18464,18643,543
500,52730,53004,53023,53344,181
1000,115594,116063,116048,116464,87
2000,251663,252387,252335,253121,41
5000,694820,695738,695894,697380,15
10000,1493657,1495738,1495565,1500882,9
20000,3198769,3203714,3202854,3209425,9
50000,8723999,8730103,8729549,8737016,9
100000,18512660,18517208,18517527,18525363,9
200000,39187241,39191789,39213929,39302155,9
500000,105398207,105470375,105492830,105713077,9
```

Self test
---------
A self test program is included. To run it you need:

* [Catch!](https://github.com/philsquared/Catch)
  - a header only unit test frame work
* [trompeloeil](https://github.com/rollbear/trompeloeil)
  - a header only mocking frame work
  