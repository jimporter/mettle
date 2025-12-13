#ifndef INC_METTLE_DRIVER_DETAIL_EXPORT_HPP
#define INC_METTLE_DRIVER_DETAIL_EXPORT_HPP

#ifdef LIBMETTLE_STATIC
#  define METTLE_PUBLIC
#  define METTLE_PUBLIC_FRIEND
#elif defined(_WIN32)
#  ifdef LIBMETTLE_EXPORTS
#    define METTLE_PUBLIC __declspec(dllexport)
#  else
#    define METTLE_PUBLIC __declspec(dllimport)
#  endif
#  define METTLE_PUBLIC_FRIEND METTLE_PUBLIC
#else
#  define METTLE_PUBLIC [[gnu::visibility("default")]]
#  define METTLE_PUBLIC_FRIEND
#endif

#endif
