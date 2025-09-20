#ifndef INC_METTLE_DETAIL_SOURCE_LOCATION_HPP
#define INC_METTLE_DETAIL_SOURCE_LOCATION_HPP

#include <concepts>
#include <version>

// Try to use `std::source_location`, except on Clang 15, where it's broken.
// See <https://github.com/llvm/llvm-project/issues/56379>.
#if defined(__cpp_lib_source_location) &&               \
    __has_include(<source_location>) &&                 \
    (!defined(__clang__) || __clang_major__ >= 16)
#  include <source_location>

namespace mettle::detail {
  using source_location = std::source_location;
}

#elif !defined(METTLE_FOUND_SOURCE_LOCATION) &&         \
      __has_include(<experimental/source_location>)
#  include <experimental/source_location>

namespace mettle::detail {
  using source_location = std::experimental::source_location;
}

#else
#  include "source_location_shim.hpp"

namespace mettle::detail {
  using source_location = source_location_shim;
}

#endif

namespace mettle {

  template<typename T>
  struct with_source_location {
    template<std::convertible_to<T> U>
    with_source_location(
      U &&val, detail::source_location loc = detail::source_location::current()
    ) : value(std::forward<U>(val)), location(loc) {}

    explicit operator T & () { return value; }

    T value;
    detail::source_location location;
  };

} // namespace mettle

#endif
