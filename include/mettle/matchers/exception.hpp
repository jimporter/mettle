#ifndef INC_METTLE_MATCHERS_EXCEPTION_HPP
#define INC_METTLE_MATCHERS_EXCEPTION_HPP

#include "core.hpp"

namespace mettle {

template<typename Exception, typename T>
auto thrown_raw(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return make_matcher([matcher](auto &&value) -> bool {
    try {
      value();
    }
    catch(const Exception &e) {
      return matcher(e);
    }
    catch(...) {}

    return false;
  }, "threw " + matcher.desc());
}

template<typename Exception, typename T>
auto thrown(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return thrown_raw<Exception>(make_matcher([matcher](auto &&value) -> bool {
    return matcher(std::string(value.what()));
  }, matcher.desc()));
}

template<typename Exception>
auto thrown() {
  return thrown_raw<Exception>(anything());
}

inline auto thrown() {
  return make_matcher([](auto &&value) -> bool {
    try {
      value();
      return false;
    }
    catch(...) {
      return true;
    }
  }, "threw");
}

} // namespace mettle

#endif
