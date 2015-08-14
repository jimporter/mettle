#ifndef INC_METTLE_DETAIL_MOVE_IF_HPP
#define INC_METTLE_DETAIL_MOVE_IF_HPP

#include <type_traits>

namespace mettle {

namespace detail {
  template<typename Container, typename Element>
  struct ref_if {
    using type = typename std::conditional<
      std::is_lvalue_reference<Container>::value,
      typename std::remove_reference<Element>::type &,
      typename std::remove_reference<Element>::type &&
     >::type;
  };

  // XXX: We could use decltype(auto) for this function, but MSVC's
  // implementation is currently broken.
  template<typename Container, typename Element>
  inline typename ref_if<Container, Element>::type
  forward_if(Element &&value) {
    return static_cast<typename ref_if<Container, Element>::type>(value);
  }
}

} // namespace mettle

#endif
