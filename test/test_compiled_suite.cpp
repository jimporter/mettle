#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"

template<typename T>
struct mock_test {
  std::string name;
  std::function<T> function;
  attributes attrs;
};

template<typename>
struct wrap_test;

template<typename Ret, typename ...Args>
struct wrap_test<Ret(Args...)> {
  template<typename Input>
  std::function<Ret(Args...)> operator ()(const Input &) {
    return [](Args...) {
      return Ret{};
    };
  }
};

template<typename ...Args>
struct wrap_test<void(Args...)> {
  template<typename Input>
  std::function<void(Args...)> operator ()(const Input &) {
    return [](Args...) {};
  }
};

template<typename T>
std::string to_printable(const basic_test_info<T> &test) {
  std::ostringstream ss;
  ss << "test_info(" << to_printable(test.name) << ", "
     << to_printable(test.attrs) << ")";
  return ss.str();
}

template<typename T>
auto equal_test_info(const T &expected) {
  std::ostringstream ss;
  ss << "test_info(" << to_printable(expected.name) << ", "
     << to_printable(expected.attrs) << ")";

  return describe(
    all(
      filter([](auto &&x) { return x.name;  }, equal_to(expected.name)),
      filter([](auto &&x) { return x.attrs; }, equal_attributes(expected.attrs))
    ), ss.str()
  );
}

suite<> test_compiled_suite("compiled_suite", [](auto &_) {
  _.test("construct", []() {
    using inner_func = void(int&);
    using outer_func = void();

    std::vector<mock_test<inner_func>> tests = {
      { "test 1", nullptr, {} },
      { "test 2", nullptr, {skip("1")} }
    };
    std::vector<compiled_suite<inner_func>> subsuites;
    compiled_suite<outer_func> compiled(
      "suite", tests, subsuites, {skip("2")}, wrap_test<outer_func>()
    );

    using compiled_test = basic_test_info<outer_func>;
    expect(compiled.name(), equal_to("suite"));
    expect(compiled.tests(), each<compiled_test>(
      {{ "test 1", nullptr, {skip("2")} },
       { "test 2", nullptr, {skip("1")} }},
      equal_test_info<compiled_test>
    ));
    expect(compiled.subsuites(), array());
  });

  _.test("construct from existing compiled_suite (copy)", []() {
    using inner_func = void(int&, int&);
    using mid_func   = void(int&);
    using outer_func = void();

    std::vector<mock_test<inner_func>> tests = {
      { "test 1", nullptr, {} },
      { "test 2", nullptr, {skip("1")} }
    };
    std::vector<compiled_suite<inner_func>> subsuites;
    compiled_suite<mid_func> suite(
      "suite", tests, subsuites, {skip("2")}, wrap_test<mid_func>()
    );
    compiled_suite<outer_func> compiled(
      suite, {skip("3")}, wrap_test<outer_func>()
    );

    using compiled_test = basic_test_info<outer_func>;
    expect(compiled.name(), equal_to("suite"));
    expect(compiled.tests(), each<compiled_test>(
      {{ "test 1", nullptr, {skip("2")} },
       { "test 2", nullptr, {skip("1")} }},
      equal_test_info<compiled_test>
    ));
    expect(compiled.subsuites(), array());
  });

  _.test("construct from existing compiled_suite (move)", []() {
    using inner_func = void(int&, int&);
    using mid_func   = void(int&);
    using outer_func = void();

    std::vector<mock_test<inner_func>> tests = {
      { "test 1", nullptr, {} },
      { "test 2", nullptr, {skip("1")} }
    };
    std::vector<compiled_suite<inner_func>> subsuites;
    compiled_suite<mid_func> suite(
      "suite", tests, subsuites, {skip("2")}, wrap_test<mid_func>()
    );
    compiled_suite<outer_func> compiled(
      std::move(suite), {skip("3")}, wrap_test<outer_func>()
    );

    using compiled_test = basic_test_info<outer_func>;
    expect(compiled.name(), equal_to("suite"));
    expect(compiled.tests(), each<compiled_test>(
      {{ "test 1", nullptr, {skip("2")} },
       { "test 2", nullptr, {skip("1")} }},
      equal_test_info<compiled_test>
    ));
    expect(compiled.subsuites(), array());
  });

  _.test("construct with subsuite", []() {
    using inner_func = void(int&, int&);
    using mid_func   = void(int&);
    using outer_func = void();

    std::vector<mock_test<inner_func>> subtests = {
      { "subtest 1", nullptr, {} },
      { "subtest 2", nullptr, {skip("1")} }
    };
    std::vector<compiled_suite<inner_func>> subsubsuites;
    std::vector<compiled_suite<mid_func>> subsuites = {
      { "subsuite", subtests, subsubsuites, {skip("2")}, wrap_test<mid_func>() }
    };

    std::vector<mock_test<mid_func>> tests = {
      { "test 1", nullptr, {} },
      { "test 2", nullptr, {skip("3")} }
    };
    compiled_suite<outer_func> compiled(
      "suite", tests, subsuites, {skip("4")}, wrap_test<outer_func>()
    );

    using compiled_test = basic_test_info<outer_func>;
    expect(compiled.name(), equal_to("suite"));
    expect(compiled.tests(), each<compiled_test>(
      {{ "test 1", nullptr, {skip("4")} },
       { "test 2", nullptr, {skip("3")} }},
      equal_test_info<compiled_test>
    ));
    expect(compiled.subsuites().size(), equal_to(1));

    auto &subsuite = compiled.subsuites()[0];
    expect(subsuite.name(), equal_to("subsuite"));
    expect(subsuite.tests(), each<compiled_test>(
      {{ "subtest 1", nullptr, {skip("2")} },
       { "subtest 2", nullptr, {skip("1")} }},
      equal_test_info<compiled_test>
    ));
    expect(subsuite.subsuites(), array());
  });
});
