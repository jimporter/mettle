#ifndef INC_METTLE_DETAIL_MOVE_IF_HPP
#define INC_METTLE_DETAIL_MOVE_IF_HPP

#include <type_traits>

namespace mettle {

namespace detail {
  template<typename Container, typename Element>
  inline decltype(auto) forward_if(Element &&value) {
    using Value = typename std::remove_reference<Element>::type;
    using ReturnType = typename std::conditional<
      std::is_lvalue_reference<Container>::value, Value &, Value &&
    >::type;
    return static_cast<ReturnType>(value);
  }
}

} // namespace mettle

#endif
