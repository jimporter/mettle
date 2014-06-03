#include <mettle.hpp>
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

  auto check_subsuite_structure = [](suites_list &suites) {
    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.subsuites().size(), equal_to<size_t>(1));

    auto &sub = inner.subsuites()[0];
    expect(sub.size(), equal_to<size_t>(2));
    expect(sub, array(
      match_test("subtest", false), match_test("skipped subtest", true)
    ));
    expect(sub.subsuites().size(), equal_to<size_t>(1));

    auto &subsub = sub.subsuites()[0];
    expect(subsub.size(), equal_to<size_t>(2));
    expect(subsub, array(
      match_test("sub-subtest", false), match_test("skipped sub-subtest", true)
    ));
    expect(subsub.subsuites().size(), equal_to<size_t>(0));
  };

  _.test("create subsuites", [&check_subsuite_structure](suites_list &suites) {

    suite<>("inner test suite", [](auto &_){
      _.template subsuite<int>("subsuite", [](auto &_) {
        _.test("subtest", [](int &) {});
        _.skip_test("skipped subtest", [](int &) {});

        _.template subsuite<>("sub-subsuite", [](auto &_) {
          _.test("sub-subtest", [](int &) {});
          _.skip_test("skipped sub-subtest", [](int &) {});
        });
      });
    }, suites);

    check_subsuite_structure(suites);
  });

  _.test("create subsuites with helper syntax",
         [&check_subsuite_structure](suites_list &suites) {

    suite<>("inner test suite", [](auto &_){
      subsuite<int>(_, "subsuite", [](auto &_) {
        _.test("subtest", [](int &) {});
        _.skip_test("skipped subtest", [](int &) {});

        subsuite<>(_, "sub-subsuite", [](auto &_) {
          _.test("sub-subtest", [](int &) {});
          _.skip_test("skipped sub-subtest", [](int &) {});
        });
      });
    }, suites);

    check_subsuite_structure(suites);
  });

});

struct basic_fixture {
  basic_fixture() = default;
  basic_fixture(const basic_fixture &) = delete;
  basic_fixture & operator =(const basic_fixture &) = delete;

  int data;
};

suite<basic_fixture> basic("suite fixtures", [](auto &_) {

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

  // Put the setup after the subsuite is created to ensure that order doesn't
  // matter.
  _.setup([](basic_fixture &f) {
    f.data = 1;
  });

});

suite<suites_list> failures("test failure states", [](auto &_) {

  _.test("teardown called when test fails", [](suites_list &suites) {
    bool teardown_called = false;
    suite<>("inner test suite", [&teardown_called](auto &_){
      _.teardown([&teardown_called]() {
        teardown_called = true;
      });

      _.test("inner test", []() {
        expect(false, equal_to(true));
      });
    }, suites);

    auto &inner = suites[0];
    for(auto &test : inner)
      test.function();

    expect(teardown_called, equal_to(true));
  });

  _.test("teardown not called when setup fails", [](suites_list &suites) {
    bool teardown_called = false;
    suite<>("inner test suite", [&teardown_called](auto &_){
      _.setup([]() {
        expect(false, equal_to(true));
      });

      _.teardown([&teardown_called]() {
        teardown_called = true;
      });

      _.test("inner test", []() {});
    }, suites);

    auto &inner = suites[0];
    for(auto &test : inner)
      test.function();

    expect(teardown_called, equal_to(false));
  });

});
