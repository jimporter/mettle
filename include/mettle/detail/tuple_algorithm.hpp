#ifndef INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP
#define INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP

#include <cstdint>
#include <ostream>
#include <tuple>
#include <type_traits>

#include "algorithm.hpp"

namespace mettle::detail {

  template<typename Func, std::size_t ...I>
  constexpr void static_for_impl(Func &&f, std::index_sequence<I...>) {
    (f(std::integral_constant<std::size_t, I>{}),...);
  }

  template<std::size_t N, typename Func>
  constexpr void static_for(Func &&f) {
    static_for_impl(std::forward<Func>(f), std::make_index_sequence<N>{});
  }

  template<std::size_t I, typename Func>
  constexpr bool static_for_while_impl(Func &&f) {
    std::integral_constant<std::size_t, I> index{};
    if constexpr(I == 0) {
      return f(index);
    } else {
      if(static_for_while_impl<I-1>(f))
        return f(index);
      return false;
    }
  }

  template<std::size_t N, typename Func>
  constexpr void static_for_while(Func &&f) {
    if constexpr(N > 0)
      static_for_while_impl<N-1>(std::forward<Func>(f));
  }

  template<typename Tuple, typename Func>
  constexpr void tuple_for_each(Tuple &&tuple, Func &&f) {
    constexpr auto N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
    static_for<N>([&](auto i) {
      return f(std::get<decltype(i)::value>(tuple));
    });
  }

  template<typename Tuple, typename Func>
  constexpr void tuple_for_each_while(Tuple &&tuple, Func &&f) {
    constexpr auto N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
    static_for_while<N>([&](auto i) {
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
