#include <mettle.hpp>
using namespace mettle;

suite<> test_factory("fixture factories", [](auto &_) {
  _.test("auto_factory", []() {
    expect(auto_factory.make<int>(), equal_to(0));
    expect(auto_factory.make<std::string>(), equal_to(""));
    expect(auto_factory.make<std::vector<int>>(), array());
  });

  subsuite(_, "bind_factory", [](auto &_) {
    _.test("no arguments", []() {
      auto f = bind_factory();
      expect(f.make<int>(), equal_to(0));
      expect(f.make<std::string>(), equal_to(""));
    });

    _.test("one argument (lvalue)", []() {
      int i = 3;
      auto f = bind_factory(i);
      i = -1;

      expect(f.make<int>(), equal_to(3));
      expect(f.make<std::vector<int>>(), array(0, 0, 0));
    });

    _.test("one argument (rvalue)", []() {
      auto f = bind_factory(3);
      expect(f.make<int>(), equal_to(3));
      expect(f.make<std::vector<int>>(), array(0, 0, 0));
    });

    _.test("two arguments (lvalue)", []() {
      int i = 3;
      char c = 'c';
      auto f = bind_factory(i, c);
      i = -1;
      c = 'x';

      expect(f.make<std::string>(), equal_to("ccc"));
      expect(f.make<std::vector<char>>(), array('c', 'c', 'c'));
    });

    _.test("two arguments (rvalue)", []() {
      auto f = bind_factory(3, 'c');
      expect(f.make<std::string>(), equal_to("ccc"));
      expect(f.make<std::vector<char>>(), array('c', 'c', 'c'));
    });

    _.test("copy/move", []() {
      auto f = bind_factory(0);
      auto g = f;
      auto h = std::move(f);

      expect(g.make<int>(), equal_to(0));
      expect(h.make<int>(), equal_to(0));
    });
  });
});
