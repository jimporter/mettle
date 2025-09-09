#ifndef INC_METTLE_DRIVER_DETAIL_EXPORT_HPP
#define INC_METTLE_DRIVER_DETAIL_EXPORT_HPP

#if defined(_WIN32) && !defined(LIBMETTLE_STATIC)
#  ifdef LIBMETTLE_EXPORTS
#    define METTLE_PUBLIC __declspec(dllexport)
#  else
#    define METTLE_PUBLIC __declspec(dllimport)
#  endif
#else
#  define METTLE_PUBLIC
#endif

#endif
