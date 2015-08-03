//
// Created by bf on 2015-07-31.
//

#ifndef TACHYMETER_SEQ_HPP
#define TACHYMETER_SEQ_HPP

#include <vector>
namespace tachymeter
{

template <template <typename> class pred, typename ... T>
struct all_are;

template <template <typename> class pred>
struct all_are<pred> : std::true_type { };

template <template <typename> class pred, typename H, typename ... Tail>
struct all_are<pred, H, Tail...>
    : public std::integral_constant<bool,
                                    pred<H>::value
                                        && all_are<pred, Tail...>::value>
{
};

class generator { };

template <typename T>
struct is_generator
{
  struct no;
  static no func(...);
  struct yes { };
  static yes            func(const generator *);
  static constexpr bool value =
                            std::is_same<decltype(func(std::declval<T *>())),
                                         yes>::value;
};

class identity_generator : public generator
{
public:
  struct iterator : public std::iterator<std::forward_iterator_tag, std::size_t>
  {
    bool operator==(iterator) const { return true; }
    bool operator!=(iterator) const { return false; }
    iterator &operator++() { return *this; }
    iterator operator++(int) { return *this; }
    value_type operator*() const { return 1; }
  };
  iterator begin() { return { }; }
  iterator end() { return { }; }
};

template <typename Generator>
class seq_t : public generator
{
public:
  template <typename G, typename ... T>
  seq_t(G &&sub_gen, T ... ts_) : gen(std::forward<G>(sub_gen)), ts{
      std::size_t(ts_)... } { }
  class iterator;
  iterator begin() { return { ts.begin(), gen.begin(), gen.end() }; }
  iterator end() { return { ts.end(), gen.begin(), gen.end() }; }
private:
  Generator                gen;
  std::vector<std::size_t> ts;
};

template <typename Generator>
class power_t : public generator
{
public:
  template <typename G>
  power_t(G &&sub_gen, std::size_t floor_, std::size_t ceil_, std::size_t mul_)
      : gen(std::forward<G>(sub_gen))
      , floor(floor_)
      , ceil(ceil_ * mul_)
      , mul(mul_)
  {
  }
  class iterator;
  iterator begin() { return { floor, mul, gen.begin(), gen.end() }; }
  iterator end() { return { ceil, mul, gen.begin(), gen.end() }; }
private:
  Generator   gen;
  std::size_t floor;
  std::size_t ceil;
  std::size_t mul;
};

template <typename Generator>
class power_t<Generator>::iterator
    : public std::iterator<std::forward_iterator_tag, std::size_t>
{
public:
  using giter = typename Generator::iterator;
  iterator(std::size_t val_, std::size_t mul_, giter begin, giter end)
      : val(val_)
      , mul(mul_)
      , g_it_(begin)
      , g_begin_(begin)
      , g_end_(end)
  {
  }
  value_type operator*() const { return val * *g_it_; }
  iterator &operator++()
  {
    if (++g_it_ == g_end_)
    {
      g_it_ = g_begin_;
      val *= mul;
    }
    return *this;
  }
  iterator operator++(int)
  {
    iterator rv{ *this };
    operator++();
    return rv;
  }
  bool operator==(iterator const &rh) const
  {
    return val == rh.val && g_it_ == rh.g_it_;
  }
  bool operator!=(iterator const &rh) const
  {
    return !(*this == rh);
  }
private:
  std::size_t val;
  std::size_t mul;
  giter       g_it_;
  giter       g_begin_;
  giter       g_end_;
};

inline power_t<identity_generator> powers(std::size_t floor,
                                          std::size_t ceil,
                                          std::size_t mul)
{
  return power_t<identity_generator>(identity_generator{ }, floor, ceil, mul);
}

template <typename G>
inline
std::enable_if_t<is_generator<std::decay_t<G>>::value, power_t<std::decay_t<G>>>
powers(G &&g, std::size_t floor, std::size_t ceil, std::size_t mul)
{
  return power_t<std::decay_t<G>>(std::forward<G>(g), floor, ceil, mul);
}

template <typename ... T>
inline
std::enable_if_t<all_are<std::is_integral, T...>::value,
                 seq_t<identity_generator>>
seq(T ... t)
{
  return seq_t<identity_generator>(identity_generator{ }, t...);
}

template <typename G, typename ... T>
inline
std::enable_if_t<
    is_generator<G>::value && all_are<std::is_integral, T...>::value,
    seq_t<std::decay_t<G>>>
seq(G &&g, T ... t)
{
  return seq_t<std::decay_t<G>>(std::forward<G>(g), t...);
}

template <typename Generator>
class seq_t<Generator>::iterator
    : public std::iterator<std::forward_iterator_tag, std::size_t>
{
public:
  using giter = typename Generator::iterator;

  iterator() = default;
  iterator(std::vector<std::size_t>::const_iterator it,
           giter g_begin,
           giter g_end)
      : it_{ it }, g_it_{ g_begin }, g_begin_{ g_begin }, g_end_{ g_end } { }
  value_type operator*() const { return *it_ * *g_it_; }
  iterator &operator++()
  {
    if (++g_it_ == g_end_)
    {
      g_it_ = g_begin_;
      ++it_;
    }
    return *this;
  }
  iterator operator++(int)
  {
    iterator rv{ *this };
    operator++();
    return rv;
  }
  bool operator==(iterator const &rv) const
  {
    return it_ == rv.it_ && g_it_ == rv.g_it_;
  }
  bool operator!=(iterator const &rv) const
  {
    return !(*this == rv);
  }
private:
  std::vector<std::size_t>::const_iterator it_;
  giter                                    g_it_;
  giter                                    g_begin_;
  giter                                    g_end_;
};

}

#endif //TACHYMETER_SEQ_HPP
