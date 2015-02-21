#include <mettle.hpp>
using namespace mettle;

suite<> test_string_output("string output", [](auto &_) {
  subsuite<>(_, "escape_string()", [](auto &_) {
    _.test("char", []() {
      expect(escape_string("text"), equal_to("\"text\""));
      expect(escape_string("\"text\""), equal_to("\"\\\"text\\\"\""));
      expect(escape_string("text\nmore"), equal_to("\"text\\nmore\""));
      expect(escape_string("\a\b\f\n\r\t\v\x1f"),
             equal_to("\"\\a\\b\\f\\n\\r\\t\\v\\x1f\""));
      expect(escape_string(std::string("text")), equal_to("\"text\""));
      expect(escape_string(std::string{'a', '\0', 'b'}), equal_to("\"a\\0b\""));

      expect(escape_string("'text'", '\''), equal_to("'\\'text\\''"));
    });

    _.test("wchar_t", []() {
      expect(escape_string(L"text"), equal_to(L"\"text\""));
      expect(escape_string(L"\"text\""), equal_to(L"\"\\\"text\\\"\""));
      expect(escape_string(L"text\nmore"), equal_to(L"\"text\\nmore\""));
      expect(escape_string(L"\a\b\f\n\r\t\v\x1f"),
             equal_to(L"\"\\a\\b\\f\\n\\r\\t\\v\\x1f\""));
      expect(escape_string(std::wstring(L"text")), equal_to(L"\"text\""));
      expect(escape_string(std::wstring{'a', '\0', 'b'}),
             equal_to(L"\"a\\0b\""));

      expect(escape_string(L"'text'", L'\''), equal_to(L"'\\'text\\''"));
    });
  });

  subsuite<>(_, "string_convert()", [](auto &_) {
    _.test("char", []() {
      std::string with_nul{'a', '\0', 'b'};
      expect(string_convert("text"), equal_to("text"));
      expect(string_convert(std::string("text")), equal_to("text"));
      expect(string_convert(std::string{'a', '\0', 'b'}),
             equal_to(std::string{'a', '\0', 'b'}));
    });

    _.test("wchar_t", []() {
      std::wstring with_nul{'a', '\0', 'b'};
      expect(string_convert(L"text"), equal_to("text"));
      expect(string_convert(std::wstring(L"text")), equal_to("text"));
      expect(string_convert(std::wstring{'a', '\0', 'b'}),
             equal_to(std::string{'a', '\0', 'b'}));
    });

    _.test("char16_t", []() {
      std::u16string with_nul{'a', '\0', 'b'};
      expect(string_convert(u"text"), equal_to("text"));
      expect(string_convert(std::u16string(u"text")), equal_to("text"));
      expect(string_convert(std::u16string{'a', '\0', 'b'}),
             equal_to(std::string{'a', '\0', 'b'}));
    });

    _.test("char32_t", []() {
      std::u32string with_nul{'a', '\0', 'b'};
      expect(string_convert(U"text"), equal_to("text"));
      expect(string_convert(std::u32string(U"text")), equal_to("text"));
      expect(string_convert(std::u32string{'a', '\0', 'b'}),
             equal_to(std::string{'a', '\0', 'b'}));
    });
  });
});
