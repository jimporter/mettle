#include <mettle.hpp>
using namespace mettle;

#include <vector>
#include <list>

template<typename T>
std::string stringify(T &&t) {
  std::stringstream s;
  s << ensure_printable(std::forward<T>(t));
  return s.str();
}

void sample_function(void) {}

suite<> output("test debug printing", [](auto &_){
  _.test("primitives", []() {
    expect(stringify(666), equal_to("666"));
    expect(stringify(2.5), equal_to("2.5"));
    expect(stringify(true), equal_to("true"));
    expect(stringify(false), equal_to("false"));
    expect(stringify(nullptr), equal_to("nullptr"));
    expect(stringify("text"), equal_to("\"text\""));
    expect(stringify(std::string("text")), equal_to("\"text\""));
  });

  _.test("iterables", []() {
    expect(stringify(std::vector<int>{}), equal_to("[]"));
    expect(stringify(std::vector<int>{1}), equal_to("[1]"));
    expect(stringify(std::vector<int>{1, 2, 3}), equal_to("[1, 2, 3]"));
    expect(stringify(std::list<int>{1, 2, 3}), equal_to("[1, 2, 3]"));

    // No test for zero-sized arrays because those are illegal in C++ (even
    // though many compilers accept them).
    int arr1[] = {1};
    expect(stringify(arr1), equal_to("[1]"));
    int arr2[] = {1, 2, 3};
    expect(stringify(arr2), equal_to("[1, 2, 3]"));
  });

  _.test("tuples", []() {
    expect(stringify(std::make_tuple()), equal_to("[]"));
    expect(stringify(std::make_tuple("foo")), equal_to("[\"foo\"]"));
    expect(stringify(std::make_tuple("foo", 1, true)),
           equal_to("[\"foo\", 1, true]"));
    expect(stringify(std::make_pair("foo", 1)), equal_to("[\"foo\", 1]"));
  });

  _.test("tuples in iterables", []() {
      using vec_0_tuple = std::vector<std::tuple<>>;
      expect(stringify(vec_0_tuple{}),
             equal_to("[]"));
      expect(stringify(vec_0_tuple{{}, {}}),
             equal_to("[[], []]"));

      using vec_1_tuple = std::vector<std::tuple<std::string>>;
      expect(stringify(vec_1_tuple{}),
             equal_to("[]"));
      expect(stringify(vec_1_tuple{ std::make_tuple("foo"),
                                    std::make_tuple("bar") }),
             equal_to("[[\"foo\"], [\"bar\"]]"));

      using vec_3_tuple = std::vector<std::tuple<std::string, int, bool>>;
      expect(stringify(vec_3_tuple{}),
             equal_to("[]"));
      expect(stringify(vec_3_tuple{ std::make_tuple("foo", 1, true),
                                    std::make_tuple("bar", 2, false) }),
             equal_to("[[\"foo\", 1, true], [\"bar\", 2, false]]"));

      using vec_pair = std::vector<std::pair<std::string, int>>;
      expect(stringify(vec_pair{}),
             equal_to("[]"));
      expect(stringify(vec_pair{{"foo", 1}, {"bar", 2}}),
             equal_to("[[\"foo\", 1], [\"bar\", 2]]"));
  });

  _.test("iterables in tuples", []() {
      using vec_str = std::vector<std::string>;
      using vec_int = std::vector<int>;

      expect(stringify(std::make_tuple( vec_str{} )), equal_to("[[]]"));
      expect(stringify(std::make_tuple( vec_str{"foo", "bar"} )),
             equal_to("[[\"foo\", \"bar\"]]"));

      expect(stringify(std::make_pair( vec_str{}, vec_int{} )),
             equal_to("[[], []]"));
      expect(stringify(std::make_pair( vec_str{"foo"}, vec_int{1, 2} )),
             equal_to("[[\"foo\"], [1, 2]]"));

      expect(stringify(std::make_tuple( vec_str{}, vec_int{}, vec_int{} )),
             equal_to("[[], [], []]"));
      expect(stringify(std::make_tuple( vec_str{"foo"}, vec_int{1, 2},
                                        vec_int{1, 2, 3} )),
             equal_to("[[\"foo\"], [1, 2], [1, 2, 3]]"));
  });

  _.test("callables", []() {
    auto not_boolean = is_not(any_of("true", "1"));

    int x = 0;
    expect(stringify([]() {}), not_boolean);
    expect(stringify([x]() {}), not_boolean);
    expect(stringify(sample_function), not_boolean);
  });

  _.test("custom types", []() {
    struct some_type {};

    // This will get a string from type_info, which could be anything...
    expect(stringify(some_type{}), anything());
  });
});
