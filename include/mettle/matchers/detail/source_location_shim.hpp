#ifndef INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_SHIM_HPP
#define INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_SHIM_HPP

#include <cstdint>

// Only GCC has the intrinsics necessary to support this shim.
#if !defined(__GNUG__) || defined(__clang__)
#  define METTLE_NO_SOURCE_LOCATION
#endif

namespace mettle {

namespace detail {

  class source_location {
  public:
    constexpr source_location() noexcept :
      file_("unknown"), func_("unknown"), line_(0), col_(0) {}

    static source_location current(
#ifdef METTLE_NO_SOURCE_LOCATION
      const char *file = "unknown",
      const char *func = "unknown",
      std::uint_least32_t line = 0,
      std::uint_least32_t col = 0
#else
      const char *file = __builtin_FILE(),
      const char *func = __builtin_FUNCTION(),
      std::uint_least32_t line = __builtin_LINE(),
      std::uint_least32_t col = 0
#endif
    ) {
      source_location loc;
      loc.file_ = file;
      loc.func_ = func;
      loc.line_ = line;
      loc.col_  = col;
      return loc;
    }

    constexpr const char* file_name() const {
      return file_;
    }

    constexpr const char* function_name() const {
      return func_;
    }

    constexpr std::uint_least32_t line() const {
      return line_;
    }

    constexpr std::uint_least32_t column() const {
      return col_;
    }
  private:
    const char *file_, *func_;
    std::uint_least32_t line_, col_;
  };

}

} // namespace mettle

#endif
