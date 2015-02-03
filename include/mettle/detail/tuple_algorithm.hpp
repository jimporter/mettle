#ifndef INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP
#define INC_METTLE_DETAIL_TUPLE_ALGORITHM_HPP

#include <cstdint>
#include <ostream>
#include <tuple>

namespace mettle {

namespace detail {

  template<std::size_t N>
  struct do_until {
    template<typename Tuple, typename Func>
    do_until(Tuple &&tuple, Func &&f) {
      using T = typename std::remove_reference<Tuple>::type;
      constexpr auto I = std::tuple_size<T>::value - N;
      if(!f( std::get<I>(tuple) ))
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

  template<typename Tuple, typename Func>
  class tuple_joiner {
  public:
    tuple_joiner(const Tuple &tuple, const Func &func, const std::string &delim)
      : tuple_(tuple), func_(func), delim_(delim) {}

    friend std::ostream & operator <<(std::ostream &os, const tuple_joiner &t) {
      std::size_t i = 0;
      tuple_for_until(t.tuple_, [&os, &i, &t](const auto &item) {
        if(i++ != 0)
          os << t.delim_;
        os << t.func_(item);
        return false;
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
}

} // namespace mettle

#endif
