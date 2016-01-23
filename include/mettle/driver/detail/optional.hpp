#ifndef INC_METTLE_DRIVER_DETAIL_OPTIONAL_HPP
#define INC_METTLE_DRIVER_DETAIL_OPTIONAL_HPP

// Get _LIBCPP_VERSION to detect libc++.
#include <ciso646>

// Try to use N4480's optional class, or fall back to Boost's.
#if defined(METTLE_USE_STDLIB_EXTS)
#  include <experimental/optional>
#  define METTLE_OPTIONAL_NS std::experimental
#elif !defined(METTLE_NO_STDLIB_EXTS) && defined(__has_include)
   // XXX: clang doesn't support SFINAE in variadic templates, which libstdc++'s
   // `optional` relies on. See <https://llvm.org/bugs/show_bug.cgi?id=23840>.
#  if !(defined(__clang__) && !defined(_LIBCPP_VERSION) && \
        defined(__GLIBCXX__)) && __has_include(<experimental/optional>)
#    include <experimental/optional>
#    define METTLE_OPTIONAL_NS std::experimental
#  else
#    include <boost/optional.hpp>
#    include <boost/optional/optional_io.hpp>
#    define METTLE_OPTIONAL_NS boost
#  endif
#else
#  include <boost/optional.hpp>
#  include <boost/optional/optional_io.hpp>
#  define METTLE_OPTIONAL_NS boost
#endif

#endif
