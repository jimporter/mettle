#ifndef INC_METTLE_MATCHERS_EXCEPTION_HPP
#define INC_METTLE_MATCHERS_EXCEPTION_HPP

#include "core.hpp"

namespace mettle {

template<typename Exception, typename T>
auto thrown_raw(T &&thing) {
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](auto &&value, auto &&matcher) -> bool {
      try {
        value();
      }
      catch(const Exception &e) {
        return matcher(e);
      }
      catch(...) {}

      return false;
    }, "threw<" + type_name<Exception>() + "> "
  );
}

template<typename Exception, typename T>
auto thrown(T &&thing) {
  return thrown_raw<Exception>(
    make_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](const auto &value, auto &&matcher) -> bool {
        return matcher(std::string(value.what()));
      }, "what: "
    )
  );
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
  }, "threw exception");
}

} // namespace mettle

#endif
