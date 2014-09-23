#ifndef INC_METTLE_SRC_LIBMETTLE_OPTIONAL_HPP
#define INC_METTLE_SRC_LIBMETTLE_OPTIONAL_HPP

// Try to use N4082's optional class, or fall back to Boost's.
#ifdef __has_include
#  if __has_include(<experimental/optional>)
#    include <experimental/optional>
#    define METTLE_OPTIONAL_NS std::experimental
#  else
#    include <boost/optional.hpp>
#    define METTLE_OPTIONAL_NS boost
#  endif
#else
#  include <boost/optional.hpp>
#  define METTLE_OPTIONAL_NS boost
#endif

#endif
