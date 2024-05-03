#include <mettle.hpp>
using namespace mettle;

#include <cstring>

template<typename Char>
std::basic_string<Char> make_string(const char *s) {
  std::size_t n = std::strlen(s);
  std::basic_string<Char> result(n, '\0');
  for(std::size_t i = 0; i != n; i++)
    result[i] = s[i];
  return result;
}

suite<> test_string_output("string output", [](auto &_) {
  subsuite<>(_, "escape_string()", [](auto &_) {
    _.test("std::string", []() {
      using namespace std::literals::string_literals;
      expect(escape_string("text"s), equal_to("\"text\""));
    });

    _.test("std::string_view", []() {
      using namespace std::literals::string_view_literals;
      expect(escape_string("text"sv), equal_to("\"text\""));
    });

    _.test("char *", []() {
      expect(escape_string("text"), equal_to("\"text\""));
    });

    _.test("escaping", []() {
      expect(escape_string("\"text\""), equal_to("\"\\\"text\\\"\""));
      expect(escape_string("te\\xt"), equal_to("\"te\\\\xt\""));
      expect(escape_string("text\nmore"), equal_to("\"text\\nmore\""));
      expect(escape_string(std::string{'a', '\0', 'b'}), equal_to("\"a\\0b\""));
      expect(escape_string("\a\b\f\n\r\t\v\x1f"),
             equal_to("\"\\a\\b\\f\\n\\r\\t\\v\\x1f\""));
    });

    _.test("delimiter", []() {
      expect(escape_string("'text'", '\''), equal_to("'\\'text\\''"));
      expect(escape_string("te\\xt", '\''), equal_to("'te\\\\xt'"));
    });
  });

  subsuite<
    char, unsigned char, signed char, wchar_t, char16_t, char32_t
  >(_, "convert_string()", type_only, [](auto &_) {
    using Char = fixture_type_t<decltype(_)>;
    auto S = &make_string<Char>;

    _.test("std::basic_string", [S]() {
      expect(convert_string(S("text")), equal_to("text"));
      expect(convert_string(std::basic_string<Char>{'a', '\0', 'b'}),
             equal_to(std::string{'a', '\0', 'b'}));
    });

    _.test("std::basic_string_view", [S]() {
      using sv = std::basic_string_view<Char>;
      expect(convert_string(sv(S("text"))), equal_to("text"));
      expect(convert_string(sv(std::basic_string<Char>{'a', '\0', 'b'})),
             equal_to(std::string{'a', '\0', 'b'}));
    });

    _.test("C string", [S]() {
      expect(convert_string(S("text").c_str()), equal_to("text"));
    });
  });

  subsuite<>(_, "represent_string()", [](auto &_) {
    _.test("char", []() {
      expect(represent_string("text"), equal_to("\"text\""));
    });

    _.test("wchar_t", []() {
      expect(represent_string(L"text"), equal_to("\"text\""));
    });

    _.test("char16_t", []() {
      expect(represent_string(u"text"), equal_to("\"text\""));
    });

    _.test("char32_t", []() {
      expect(represent_string(U"text"), equal_to("\"text\""));
    });

    _.test("int", []() {
      expect(represent_string(std::basic_string<int>{'t', 'e', 'x', 't'}),
             equal_to("(unrepresentable string)"));
    });
  });
});
