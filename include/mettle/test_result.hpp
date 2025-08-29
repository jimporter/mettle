#ifndef INC_METTLE_TEST_RESULT_HPP
#define INC_METTLE_TEST_RESULT_HPP

#include <cstdint>
#include <optional>
#include <string>

#if __has_include(<bencode.hpp>)
#  include <bencode.hpp>
#  include "detail/forward_like.hpp"
#endif

namespace mettle {

  struct test_failure {
    std::string desc = "";
    std::string message;
    std::string file_name = "";
    std::uint_least32_t line = 0;

#if __has_include(<bencode.hpp>)
    template<typename T = bencode::data>
    auto to_bencode() const {
      return typename T::dict{
        {"desc", desc},
        {"message", message},
        {"file_name", file_name},
        {"line", line}
      };
    }

    template<typename T>
    static test_failure from_bencode(T &&data) {
      using data_t = std::remove_cvref_t<T>;
      using string_t = typename data_t::string;
      using integer_t = typename data_t::integer;

      auto &dict = std::get<typename data_t::dict>(data);
      return {
        detail::forward_like<T>(std::get<string_t>(dict.at("desc"))),
        detail::forward_like<T>(std::get<string_t>(dict.at("message"))),
        detail::forward_like<T>(std::get<string_t>(dict.at("file_name"))),
        static_cast<std::uint_least32_t>(std::get<integer_t>(dict.at("line")))
      };
    }
#endif
  };

  using test_result = std::optional<test_failure>;

} // namespace mettle

#endif
