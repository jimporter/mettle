#include <mettle.hpp>
using namespace mettle;

#include <vector>
#include <list>

suite<std::vector<int>, std::list<int>> param("test collections", [](auto &_) {

  subsuite<>(_, "empty collection", [](auto &_) {
    _.test("size()", [](auto &coll) {
      expect(coll.size(), equal_to(0u));
    });

    _.test("empty()", [](auto &coll) {
      expect(coll.empty(), equal_to(true));
    });

    _.test("begin() == end()", [](auto &coll) {
      expect(coll.begin(), equal_to(coll.end()));
    });
  });

  subsuite<>(_, "non-empty collection", [](auto &_) {
    _.setup([](auto &coll) {
      coll.push_back(1);
      coll.push_back(0);
      coll.push_back(2);
    });

    _.test("size()", [](auto &coll) {
      expect(coll.size(), equal_to(3u));
    });

    _.test("empty()", [](auto &coll) {
      expect(coll.empty(), equal_to(false));
    });
  });

  subsuite<int, float>(_, "emplacement", [](auto &_) {
    _.setup([](auto &, auto &elt) {
      elt = 1;
    });

    _.test("emplace_back()", [](auto &coll, auto &elt) {
      coll.emplace_back(elt);
      expect(coll.back(), equal_to(1));
    });
  });

});
