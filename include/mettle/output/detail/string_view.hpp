#ifndef INC_METTLE_OUTPUT_DETAIL_STRING_VIEW_HPP
#define INC_METTLE_OUTPUT_DETAIL_STRING_VIEW_HPP

// Try to use std::string_view, N4480's version, or fall back to Boost's.

#ifdef __has_include
#  if __has_include(<string_view>) && (__cplusplus >= 201703L || \
      (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
#    include <string_view>
#    define METTLE_STRING_VIEW std::basic_string_view
#  elif __has_include(<experimental/string_view>) && \
        !defined(METTLE_NO_STDLIB_EXTS)
#    include <experimental/string_view>
#    define METTLE_STRING_VIEW std::experimental::basic_string_view
#  endif
#endif

#ifndef METTLE_STRING_VIEW
#  ifdef METTLE_USE_STDLIB_EXTS
#    include <experimental/string_view>
#    define METTLE_STRING_VIEW std::experimental::basic_string_view
#  else
#    include <boost/version.hpp>
#    if BOOST_VERSION >= 106100
#      include <boost/utility/string_view.hpp>
#      define METTLE_STRING_VIEW boost::basic_string_view
#    else
#      include <boost/utility/string_ref.hpp>
#      define METTLE_STRING_VIEW boost::basic_string_ref
#    endif
#  endif
#endif

#endif
