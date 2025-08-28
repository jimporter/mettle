#ifndef INC_METTLE_DETAIL_SOURCE_LOCATION_SHIM_HPP
#define INC_METTLE_DETAIL_SOURCE_LOCATION_SHIM_HPP

#include <cstdint>

#ifdef __has_builtin
#  if !__has_builtin(__builtin_FILE)
#    define METTLE_NO_SOURCE_LOCATION
#  endif
#else
#  define METTLE_NO_SOURCE_LOCATION
#endif

namespace mettle::detail {

  class source_location_shim {
  public:
    constexpr source_location_shim() noexcept :
      file_name_("unknown"), func_name_("unknown"), line_(0), col_(0) {}

    static source_location_shim current(
#ifdef METTLE_NO_SOURCE_LOCATION
      const char *file_name = "unknown",
      const char *func_name = "unknown",
      std::uint_least32_t line = 0,
      std::uint_least32_t col = 0
#else
      const char *file_name = __builtin_FILE(),
      const char *func_name = __builtin_FUNCTION(),
      std::uint_least32_t line = __builtin_LINE(),
      std::uint_least32_t col = 0
#endif
    ) {
      source_location_shim loc;
      loc.file_name_ = file_name;
      loc.func_name_ = func_name;
      loc.line_ = line;
      loc.col_  = col;
      return loc;
    }

    constexpr const char* file_name() const {
      return file_name_;
    }

    constexpr const char* function_name() const {
      return func_name_;
    }

    constexpr std::uint_least32_t line() const {
      return line_;
    }

    constexpr std::uint_least32_t column() const {
      return col_;
    }
  private:
    const char *file_name_, *func_name_;
    std::uint_least32_t line_, col_;
  };

} // namespace mettle::detail

#endif
