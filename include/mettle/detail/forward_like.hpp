#ifndef INC_METTLE_DETAIL_FORWARD_LIKE_HPP
#define INC_METTLE_DETAIL_FORWARD_LIKE_HPP

#include <type_traits>
#include <utility>

namespace mettle::detail {
  // TODO: Remove this once we require C++23.
  template<typename T, typename U>
  inline auto && forward_like(U &&value) {
    using Value = std::remove_reference_t<U>;
    using Ref = std::conditional_t<
      std::is_lvalue_reference_v<T>, Value &, Value &&
    >;
    using CRef = std::conditional_t<
      std::is_const_v<std::remove_reference_t<T>>, const Ref, Ref
    >;
    return static_cast<CRef>(value);
  }
} // namespace mettle::detail

#endif
