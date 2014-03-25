#include "mettle.hpp"
using namespace mettle;

inline auto match_test(const std::string &name, bool skip) {
  std::stringstream s;
  if(skip)
    s << "skipped ";
  s << "test named \"" << name << "\"";
  return make_matcher([name, skip](const runnable_suite::test_info &actual) {
    return actual.name == name && actual.skip == skip;
  }, s.str());
}

suite<suites_list> test_suite("suite creation", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<>("inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int>("inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with setup/teardown", [](suites_list &suites) {
    suite<>("inner test suite", [](auto &_){
      _.setup([]() {});
      _.teardown([]() {});
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with fixture and setup/teardown",
         [](suites_list &suites) {
    suite<int>("inner test suite", [](auto &_){
      _.setup([](int &) {});
      _.teardown([](int &) {});
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite that throws", [](suites_list &suites) {
    try {
      suite<int>("broken test suite", [](auto &){
        throw "bad";
      }, suites);
    } catch(...) {}

    expect(suites.size(), equal_to<size_t>(0));
  });

});

struct basic_fixture {
  basic_fixture() = default;
  basic_fixture(const basic_fixture &) = delete;
  basic_fixture & operator =(const basic_fixture &) = delete;

  int data;
};

suite<basic_fixture> basic("nested suites", [](auto &_) {
  _.template subsuite<>("subsuite", [](auto &_) {
    _.setup([](basic_fixture &f) {
      f.data++;
    });

    _.test("fixture was passed by reference", [](basic_fixture &f) {
      expect(f.data, equal_to(2));
    });

    _.template subsuite<int>("sub-subsuite", [](auto &_) {
      _.setup([](basic_fixture &f, int &) {
        f.data++;
      });

      _.test("fixture was passed by reference", [](basic_fixture &f, int &) {
        expect(f.data, equal_to(3));
      });
    });

  });

  _.setup([](basic_fixture &f) {
    f.data = 1;
  });
});
