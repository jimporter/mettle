#include <mettle.hpp>
using namespace mettle;

#include <vector>
#include <list>

template<typename T>
auto stringified(T &&thing) {
  return basic_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) -> match_result {
      std::ostringstream ss;
      ss << to_printable(value);
      return {matcher(ss.str()), to_printable(ss.str())};
    }, ""
  );
}

auto stringified_ptr = stringified(regex_match("(0x)?[0-9a-fA-F]+"));

struct my_type {};
std::string to_printable(const my_type &) {
  return "{my_type}";
}

namespace my_namespace {
  struct another_type {};
  std::string to_printable(const another_type &) {
    return "{another_type}";
  }
}

struct unprintable_type {};

struct unprintable_bool_type {
  operator bool() { return true; }
};

enum my_enum {
  enum_value = 0
};

template<>
struct std::char_traits<my_enum> {
  using char_type = my_enum;

  static void assign(char_type& c1, const char_type& c2) {
    c1 = c2;
  }

  static char_type * assign(char_type *ptr, std::size_t count, char_type c) {
    for (std::size_t i = 0; i != count; i++)
      ptr[i] = c;
    return ptr;
  }

  static char_type *
  copy(char_type *dest, const char_type *src, std::size_t count) {
    for (std::size_t i = 0; i != count; i++)
      dest[i] = src[i];
    return dest;
  }

  static char_type *
  move(char_type *dest, const char_type *src, std::size_t count) {
    for (std::size_t i = 0; i != count; i++)
      dest[i] = src[i];
    return dest;
  }
};

enum class my_enum_class {
  value = 0
};

void sample_function(void) {}

suite<> test_to_printable("to_printable()", [](auto &_) {
  _.test("integers", []() {
    expect(666, stringified("666"));
    expect(666L, stringified("666"));
    expect(666LL, stringified("666"));

    expect(666U, stringified("666"));
    expect(666UL, stringified("666"));
    expect(666ULL, stringified("666"));

    int a = 666;
    expect(a, stringified("666"));
    expect(const_cast<const int&>(a), stringified("666"));
    expect(const_cast<const volatile int&>(a), stringified("666"));
  });

  _.test("floats", []() {
    expect(2.5, stringified("2.5"));
    expect(2.5f, stringified("2.5"));
  });

  _.test("booleans", []() {
    expect(true, stringified("true"));
    expect(false, stringified("false"));
  });

  _.test("enums", []() {
    expect(enum_value, stringified(regex_match("(enum )?my_enum\\(0\\)$")));
    expect(my_enum_class::value, stringified(
      regex_match("(enum )?my_enum_class\\(0\\)$")
    ));
  });

  _.test("pointers", []() {
    expect(nullptr, stringified("nullptr"));

    void *x = 0;
    expect(x, stringified("nullptr"));
    expect(const_cast<const void*>(x), stringified("nullptr"));

    int *y = 0;
    expect(y, stringified("nullptr"));
    expect(const_cast<const int*>(y), stringified("nullptr"));

    struct some_type {};
    expect(some_type{}, stringified(none("true", "1")));
  });

  _.test("iterables", []() {
    expect(std::vector<int>{}, stringified("[]"));
    expect(std::vector<int>{1}, stringified("[1]"));
    expect(std::vector<int>{1, 2, 3}, stringified("[1, 2, 3]"));
    expect(std::list<int>{1, 2, 3}, stringified("[1, 2, 3]"));

    // No test for zero-sized arrays because those are illegal in C++ (even
    // though many compilers accept them).
    int arr1[] = {1};
    expect(arr1, stringified("[1]"));
    int arr2[] = {1, 2, 3};
    expect(arr2, stringified("[1, 2, 3]"));
  });

  _.test("tuples", []() {
    expect(std::make_tuple(), stringified("[]"));
    expect(std::make_tuple("foo"), stringified("[\"foo\"]"));
    expect(std::make_tuple("foo", 1, true),
           stringified("[\"foo\", 1, true]"));
    expect(std::make_pair("foo", 1), stringified("[\"foo\", 1]"));
  });

  _.test("tuples in iterables", []() {
    using vec_0_tuple = std::vector<std::tuple<>>;
    expect(vec_0_tuple{}, stringified("[]"));
    expect(vec_0_tuple{{}, {}}, stringified("[[], []]"));

    using vec_1_tuple = std::vector<std::tuple<std::string>>;
    expect(vec_1_tuple{}, stringified("[]"));
    expect(vec_1_tuple{ std::make_tuple("foo"), std::make_tuple("bar") },
           stringified("[[\"foo\"], [\"bar\"]]"));

    using vec_3_tuple = std::vector<std::tuple<std::string, int, bool>>;
    expect(vec_3_tuple{}, stringified("[]"));
    expect(vec_3_tuple{ std::make_tuple("foo", 1, true),
        std::make_tuple("bar", 2, false) },
    stringified("[[\"foo\", 1, true], [\"bar\", 2, false]]"));

    using vec_pair = std::vector<std::pair<std::string, int>>;
    expect(vec_pair{}, stringified("[]"));
    expect(vec_pair{{"foo", 1}, {"bar", 2}},
           stringified("[[\"foo\", 1], [\"bar\", 2]]"));
  });

  _.test("iterables in tuples", []() {
    using vec_str = std::vector<std::string>;
    using vec_int = std::vector<int>;

    expect(std::make_tuple( vec_str{} ), stringified("[[]]"));
    expect(std::make_tuple( vec_str{"foo", "bar"} ),
           stringified("[[\"foo\", \"bar\"]]"));

    expect(std::make_pair( vec_str{}, vec_int{} ),
           stringified("[[], []]"));
    expect(std::make_pair( vec_str{"foo"}, vec_int{1, 2} ),
           stringified("[[\"foo\"], [1, 2]]"));

    expect(std::make_tuple( vec_str{}, vec_int{}, vec_int{} ),
           stringified("[[], [], []]"));
    expect(std::make_tuple( vec_str{"foo"}, vec_int{1, 2},
                            vec_int{1, 2, 3} ),
           stringified("[[\"foo\"], [1, 2], [1, 2, 3]]"));
  });

  _.test("callables", []() {
    auto not_boolean = stringified(none("true", "1"));

    int x = 0;
    expect([]() {}, not_boolean);
    expect([x]() mutable { x = 1; }, not_boolean);
    expect(sample_function, stringified(type_name<void(void)>()));
  });

  _.test("optional", []() {
    std::optional<std::string> empty;
    expect(empty, stringified("std::nullopt"));
    std::optional<std::string> filled{"hello"};
    expect(filled, stringified("std::optional(\"hello\")"));
  });

  _.test("exceptions", []() {
    std::runtime_error e("what");
    expect(e, stringified(type_name<std::runtime_error>() + "(\"what\")"));
    expect(static_cast<std::exception&>(e),
           stringified(type_name<std::runtime_error>() + "(\"what\")"));
  });

  _.test("custom types", []() {
    expect(my_type{}, stringified("{my_type}"));
    expect(my_namespace::another_type{}, stringified("{another_type}"));

    expect(std::vector<my_type>(2), stringified("[{my_type}, {my_type}]"));
    expect(std::vector<my_namespace::another_type>(2),
           stringified("[{another_type}, {another_type}]"));

    expect(std::pair<my_type, my_type>{},
           stringified("[{my_type}, {my_type}]"));
    expect(std::pair<my_namespace::another_type,
                     my_namespace::another_type>{},
           stringified("[{another_type}, {another_type}]"));

    expect(std::tuple<my_type, my_type>{},
           stringified("[{my_type}, {my_type}]"));
    expect(std::tuple<my_namespace::another_type,
                      my_namespace::another_type>{},
           stringified("[{another_type}, {another_type}]"));

    expect(std::optional<my_type>{my_type{}},
           stringified("std::optional({my_type})"));
    expect(std::optional<my_namespace::another_type>{
      my_namespace::another_type{}
    }, stringified("std::optional({another_type})"));
  });

  _.test("unprintable types", []() {
    std::string up_ex = "(struct )?unprintable_type";
    std::string upb_ex = "(struct )?unprintable_bool_type";

    expect(unprintable_type{}, stringified(regex_match(up_ex + "$")));
    expect(unprintable_bool_type{}, stringified(regex_match(upb_ex + "$")));

    auto match_unprintables = stringified(regex_match(
      "\\[" + up_ex + ", " + up_ex + "\\]$"
    ));
    auto match_unprintable_bools = stringified(regex_match(
      "\\[" + upb_ex + ", " + upb_ex + "\\]$"
    ));

    expect(std::vector<unprintable_type>(2), match_unprintables);
    expect(std::vector<unprintable_bool_type>(2), match_unprintable_bools);

    expect(std::pair<unprintable_type, unprintable_type>{}, match_unprintables);
    expect(std::pair<unprintable_bool_type, unprintable_bool_type>{},
           match_unprintable_bools);

    expect(std::tuple<unprintable_type, unprintable_type>{},
           match_unprintables);
    expect(std::tuple<unprintable_bool_type, unprintable_bool_type>{},
           match_unprintable_bools);

    expect(std::optional<unprintable_type>{unprintable_type{}},
           stringified(regex_match("std::optional\\(" + up_ex + "\\)$")));
    expect(std::optional<unprintable_bool_type>{unprintable_bool_type{}},
           stringified(regex_match("std::optional\\(" + upb_ex + "\\)$")));
  });

  // Lots of code duplication here, but sadly, it's not easy to make generic
  // string literals...
  subsuite<>(_, "strings", [](auto &_) {
    _.test("char", []() {
      expect('x', stringified("'x'"));
      expect('\0', stringified("'\\0'"));
      expect('\n', stringified("'\\n'"));
      expect('\x1f', stringified("'\\x1f'"));

      expect("text", stringified("\"text\""));
      expect("text\nmore", stringified("\"text\\nmore\""));
      expect(std::string("text"), stringified("\"text\""));
      expect(std::string_view("text"), stringified("\"text\""));
      expect(std::string{'a', '\0', 'b'}, stringified("\"a\\0b\""));

      char s[] = "text";
      expect(s, stringified("\"text\""));
      expect(static_cast<char*>(s), stringified("\"text\""));

      const char cs[] = "text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const char*>(cs), stringified("\"text\""));

      char *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("wchar_t", []() {
      expect(L'x', stringified("'x'"));
      expect(L'\n', stringified("'\\n'"));
      expect(L'\0', stringified("'\\0'"));
      expect(L'\x1f', stringified("'\\x1f'"));

      expect(L"text", stringified("\"text\""));
      expect(L"text\nmore", stringified("\"text\\nmore\""));
      expect(std::wstring(L"text"), stringified("\"text\""));
      expect(std::wstring_view(L"text"), stringified("\"text\""));
      expect(std::wstring{L'a', L'\0', L'b'}, stringified("\"a\\0b\""));

      wchar_t s[] = L"text";
      expect(s, stringified("\"text\""));
      expect(static_cast<wchar_t*>(s), stringified("\"text\""));

      const wchar_t cs[] = L"text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const wchar_t*>(cs), stringified("\"text\""));

      wchar_t *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("char8_t", []() {
      expect(u8'x', stringified("'x'"));
      expect(u8'\n', stringified("'\\n'"));
      expect(u8'\0', stringified("'\\0'"));
      expect(u8'\x1f', stringified("'\\x1f'"));

      expect(u8"text", stringified("\"text\""));
      expect(u8"text\nmore", stringified("\"text\\nmore\""));
      expect(std::u8string(u8"text"), stringified("\"text\""));
      expect(std::u8string_view(u8"text"), stringified("\"text\""));
      expect(std::u8string{u8'a', u8'\0', u8'b'}, stringified("\"a\\0b\""));

      char8_t s[] = u8"text";
      expect(s, stringified("\"text\""));
      expect(static_cast<char8_t*>(s), stringified("\"text\""));

      const char8_t cs[] = u8"text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const char8_t*>(cs), stringified("\"text\""));

      char8_t *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("char16_t", []() {
      expect(u'x', stringified("'x'"));
      expect(u'\n', stringified("'\\n'"));
      expect(u'\0', stringified("'\\0'"));
      expect(u'\x1f', stringified("'\\x1f'"));

      expect(u"text", stringified("\"text\""));
      expect(u"text\nmore", stringified("\"text\\nmore\""));
      expect(std::u16string(u"text"), stringified("\"text\""));
      expect(std::u16string_view(u"text"), stringified("\"text\""));
      expect(std::u16string{u'a', u'\0', u'b'}, stringified("\"a\\0b\""));

      char16_t s[] = u"text";
      expect(s, stringified("\"text\""));
      expect(static_cast<char16_t*>(s), stringified("\"text\""));

      const char16_t cs[] = u"text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const char16_t*>(cs), stringified("\"text\""));

      char16_t *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("char32_t", []() {
      expect(U'x', stringified("'x'"));
      expect(U'\n', stringified("'\\n'"));
      expect(U'\0', stringified("'\\0'"));
      expect(U'\x1f', stringified("'\\x1f'"));

      expect(U"text", stringified("\"text\""));
      expect(U"text\nmore", stringified("\"text\\nmore\""));
      expect(std::u32string(U"text"), stringified("\"text\""));
      expect(std::u32string_view(U"text"), stringified("\"text\""));
      expect(std::u32string{U'a', U'\0', U'b'}, stringified("\"a\\0b\""));

      char32_t s[] = U"text";
      expect(s, stringified("\"text\""));
      expect(static_cast<char32_t*>(s), stringified("\"text\""));

      const char32_t cs[] = U"text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const char32_t*>(cs), stringified("\"text\""));

      char32_t *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("unsigned char", []() {
      expect(static_cast<unsigned char>('x'), stringified("0x78"));
      expect(static_cast<unsigned char>('\0'), stringified("0x00"));
      expect(static_cast<unsigned char>('\x1f'), stringified("0x1f"));

      unsigned char s[] = "text";
      expect(s, stringified("[0x74, 0x65, 0x78, 0x74, 0x00]"));
      expect(static_cast<unsigned char*>(s), stringified_ptr);

      const unsigned char cs[] = "text";
      expect(cs, stringified("[0x74, 0x65, 0x78, 0x74, 0x00]"));
      expect(static_cast<const unsigned char*>(cs), stringified_ptr);

      unsigned char *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("signed char", []() {
      expect(static_cast<signed char>('x'), stringified("+0x78"));
      expect(static_cast<signed char>('\0'), stringified("+0x00"));
      expect(static_cast<signed char>('\x1f'), stringified("+0x1f"));

      signed char s[] = "text";
      expect(s, stringified("[+0x74, +0x65, +0x78, +0x74, +0x00]"));
      expect(static_cast<signed char*>(s), stringified_ptr);

      const signed char cs[] = "text";
      expect(cs, stringified("[+0x74, +0x65, +0x78, +0x74, +0x00]"));
      expect(static_cast<const signed char*>(cs), stringified_ptr);

      signed char *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("std::byte", []() {
      expect(static_cast<std::byte>('x'), stringified("0x78"));
      expect(static_cast<std::byte>('\0'), stringified("0x00"));
      expect(static_cast<std::byte>('\x1f'), stringified("0x1f"));

      std::byte s[] = {std::byte('t'), std::byte('e'), std::byte('x'),
                       std::byte('t'), std::byte('\0')};
      expect(s, stringified("[0x74, 0x65, 0x78, 0x74, 0x00]"));
      expect(static_cast<std::byte*>(s), stringified_ptr);

      const std::byte cs[] = {std::byte('t'), std::byte('e'), std::byte('x'),
                              std::byte('t'), std::byte('\0')};
      expect(cs, stringified("[0x74, 0x65, 0x78, 0x74, 0x00]"));
      expect(static_cast<const std::byte*>(cs), stringified_ptr);

      std::byte *ns = nullptr;
      expect(ns, stringified("nullptr"));
    });

    _.test("custom character type", []() {
      auto string_match = stringified(
        regex_match("\\[.+\\(1\\), .+\\(2\\), .+\\(3\\)\\]")
      );

      std::basic_string<my_enum> s = {my_enum(1), my_enum(2), my_enum(3)};
      expect(s, string_match);

      std::basic_string_view<my_enum> sv = s;
      expect(sv, string_match);
    });
  });
});
