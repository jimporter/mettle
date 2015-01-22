#ifndef INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP
#define INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP

#include <cstdint>
#include <tuple>

namespace mettle {

namespace detail {

  template<std::size_t N>
  struct do_until {
    template<typename Tuple, typename Func>
    do_until(Tuple &&tuple, Func &&f) {
      using T = typename std::remove_reference<Tuple>::type;
      constexpr auto i = std::tuple_size<T>::value - N;
      if(!std::forward<Func>(f)( std::get<i>(std::forward<Tuple>(tuple)) ))
        do_until<N-1>(std::forward<Tuple>(tuple), std::forward<Func>(f));
    }
  };

  template<>
  struct do_until<0> {
    template<typename Tuple, typename Func>
    do_until(Tuple &&, Func &&) {}
  };

  template<typename Tuple, typename Func>
  void tuple_for_until(Tuple &&tuple, Func &&f) {
    using T = typename std::remove_reference<Tuple>::type;
    do_until<std::tuple_size<T>::value>(
      std::forward<Tuple>(tuple), std::forward<Func>(f)
    );
  }

  template<std::size_t N>
  struct do_each {
    template<typename Tuple, typename Func>
    do_each(Tuple &&tuple, Func &&f) {
      using T = typename std::remove_reference<Tuple>::type;
      constexpr auto i = std::tuple_size<T>::value - N;
      std::forward<Func>(f)( std::get<i>(std::forward<Tuple>(tuple)) );
      do_each<N-1>(std::forward<Tuple>(tuple), std::forward<Func>(f));
    }
  };

  template<>
  struct do_each<0> {
    template<typename Tuple, typename Func>
    do_each(Tuple &&, Func &&) {}
  };

  template<typename Tuple, typename Func>
  void tuple_for_each(Tuple &&tuple, Func &&f) {
    using T = typename std::remove_reference<Tuple>::type;
    do_each<std::tuple_size<T>::value>(
      std::forward<Tuple>(tuple), std::forward<Func>(f)
    );
  }
}

} // namespace mettle

#endif
