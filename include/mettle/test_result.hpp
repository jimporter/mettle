#ifndef INC_METTLE_TEST_RESULT_HPP
#define INC_METTLE_TEST_RESULT_HPP

#include <optional>
#include <string>

#if __has_include(<bencode.hpp>)
#  include <bencode.hpp>
#  include "detail/forward_like.hpp"
#endif

namespace mettle {

  struct test_failure {
    std::string message;

#if __has_include(<bencode.hpp>)
    template<typename T = bencode::data>
    auto to_bencode() const {
      return typename T::dict{
        {"message", message}
      };
    }

    template<typename T>
    static test_failure from_bencode(T &&data) {
      using data_t = std::remove_cvref_t<T>;
      using string_t = typename data_t::string;
      auto &dict = std::get<typename data_t::dict>(data);
      return {
        detail::forward_like<T>(std::get<string_t>(dict.at("message")))
      };
    }
#endif
  };

  using test_result = std::optional<test_failure>;

} // namespace mettle

#endif
