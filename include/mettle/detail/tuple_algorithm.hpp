#ifndef INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP
#define INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP

#include <cstdint>
#include <ostream>
#include <tuple>
#include <type_traits>

#include "string_algorithm.hpp"

namespace mettle::detail {

  template<std::size_t I, std::size_t N>
  struct do_iterate {
    template<typename Func>
    do_iterate(Func &&f) {
      constexpr std::integral_constant<std::size_t, I> index{};
      if constexpr(std::is_same_v<decltype(f(index)), void>) {
        f(index);
        do_iterate<I+1, N>(std::forward<Func>(f));
      } else {
        if(!f(index))
          do_iterate<I+1, N>(std::forward<Func>(f));
      }
    }
  };

  template<std::size_t N>
  struct do_iterate<N, N> {
    do_iterate(...) {}
  };

  template<std::size_t N>
  using static_for = do_iterate<0, N>;

  template<typename Tuple, typename Func>
  void tuple_for_each(Tuple &&tuple, Func &&f) {
    using T = typename std::remove_reference<Tuple>::type;
    static_for<std::tuple_size<T>::value>([&](auto i) {
      return f(std::get<decltype(i)::value>(tuple));
    });
  }

  template<typename Tuple, typename Func>
  class tuple_joiner {
  public:
    tuple_joiner(const Tuple &tuple, const Func &func, const std::string &delim)
      : tuple_(tuple), func_(func), delim_(delim) {}

    friend std::ostream & operator <<(std::ostream &os, const tuple_joiner &t) {
      ostream_list_append ola(os, t.delim_);
      tuple_for_each(t.tuple_, [&ola, &f = t.func_](const auto &i) {
        ola(f(i));
      });
      return os;
    }
  private:
    const Tuple &tuple_;
    const Func &func_;
    const std::string &delim_;
  };

  template<typename Tuple, typename Func>
  inline tuple_joiner<Tuple, Func>
  tuple_joined(const Tuple &tuple, const Func &func,
               const std::string &delim = ", ") {
    return {tuple, func, delim};
  }

} // namespace mettle::detail

#endif
