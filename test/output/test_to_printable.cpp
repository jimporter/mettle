#include <mettle.hpp>
using namespace mettle;

#include <vector>
#include <list>

template<typename T>
auto stringified(T &&thing) {
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) -> bool {
      std::ostringstream ss;
      ss << to_printable(value);
      return matcher(ss.str());
    }, ""
  );
}

auto nil() {
  std::ostringstream ss;
  ss << static_cast<void*>(0);
  return stringified(ss.str());
}

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

enum my_enum {
  enum_value = 0
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
    expect(x, nil());
    expect(const_cast<const void*>(x), nil());

    int *y = 0;
    expect(y, nil());
    expect(const_cast<const int*>(y), nil());

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

  _.test("exceptions", []() {
    std::runtime_error e("what");
    expect(e, stringified(type_name<std::runtime_error>() + "(\"what\")"));
    expect(static_cast<std::exception&>(e),
           stringified(type_name<std::runtime_error>() + "(\"what\")"));
  });

  _.test("custom types", []() {
    expect(my_type{}, stringified("{my_type}"));
    expect(my_namespace::another_type{}, stringified("{another_type}"));
    expect(unprintable_type{}, stringified(regex_match(
      "(struct )?unprintable_type$"
    )));

    auto match_unprintable = stringified(regex_match(
      "\\[(struct )?unprintable_type, (struct )?unprintable_type\\]$"
    ));
    expect(std::vector<my_type>(2), stringified("[{my_type}, {my_type}]"));
    expect(std::vector<my_namespace::another_type>(2),
           stringified("[{another_type}, {another_type}]"));
    expect(std::vector<unprintable_type>(2), match_unprintable);

    expect(std::pair<my_type, my_type>{},
           stringified("[{my_type}, {my_type}]"));
    expect(std::pair<my_namespace::another_type,
                     my_namespace::another_type>{},
           stringified("[{another_type}, {another_type}]"));
    expect(std::pair<unprintable_type, unprintable_type>{}, match_unprintable);

    expect(std::tuple<my_type, my_type>{},
           stringified("[{my_type}, {my_type}]"));
    expect(std::tuple<my_namespace::another_type,
                      my_namespace::another_type>{},
           stringified("[{another_type}, {another_type}]"));
    expect(std::tuple<unprintable_type, unprintable_type>{}, match_unprintable);
  });

  _.test("fallback", []() {
    struct some_type {};
    struct another_type {
      operator bool() { return true; }
    };

    // This will get a string from type_info, which could be anything...
    expect(some_type{}, anything());
    expect(another_type{}, stringified(none("true", "1")));
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
      expect(ns, nil());
    });

    _.test("signed char", []() {
      expect(static_cast<signed char>('x'), stringified("'x'"));
      expect(static_cast<signed char>('\n'), stringified("'\\n'"));
      expect(static_cast<signed char>('\0'), stringified("'\\0'"));
      expect(static_cast<signed char>('\x1f'), stringified("'\\x1f'"));

      signed char s[] = "text";
      expect(s, stringified("\"text\""));
      expect(static_cast<signed char*>(s), stringified("\"text\""));

      const signed char cs[] = "text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const signed char*>(cs), stringified("\"text\""));

      signed char *ns = nullptr;
      expect(ns, nil());
    });

    _.test("unsigned char", []() {
      expect(static_cast<unsigned char>('x'), stringified("'x'"));
      expect(static_cast<unsigned char>('\n'), stringified("'\\n'"));
      expect(static_cast<unsigned char>('\0'), stringified("'\\0'"));
      expect(static_cast<unsigned char>('\x1f'), stringified("'\\x1f'"));

      unsigned char s[] = "text";
      expect(s, stringified("\"text\""));
      expect(static_cast<unsigned char*>(s), stringified("\"text\""));

      const unsigned char cs[] = "text";
      expect(cs, stringified("\"text\""));
      expect(static_cast<const unsigned char*>(cs), stringified("\"text\""));

      unsigned char *ns = nullptr;
      expect(ns, nil());
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
      expect(ns, nil());
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
      expect(ns, nil());
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
      expect(ns, nil());
    });
  });
});
