#include <mettle.hpp>
using namespace mettle;

#include "copy_counter.hpp"

suite<> test_forward_like("forward_like()", [](auto &_) {
  subsuite<int, int &, const int &>(_, "forward_like", type_only, [](auto &_) {
    using Fixture = fixture_type_t<decltype(_)>;

    _.test("forward_like<T>(moveable)", []() {
      bool should_move = !std::is_lvalue_reference<Fixture>::value;
      moveable_type from;
      auto to = detail::forward_like<Fixture>(from);

      expect("number of copies", to.copies, equal_to(should_move ? 0 : 1));
      expect("number of moves", to.moves, equal_to(should_move ? 1 : 0));
    });

    _.test("forward_like<T>(copyable)", []() {
      copyable_type from;
      auto to = detail::forward_like<Fixture>(from);

      expect("number of copies", to.copies, equal_to(1));
    });
  });
});
