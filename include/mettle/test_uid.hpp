#ifndef INC_METTLE_TEST_UID_HPP
#define INC_METTLE_TEST_UID_HPP

#include <atomic>
#include <cstdint>

namespace mettle {

  using test_uid = std::uint64_t;

  namespace detail {
    inline std::atomic<test_uid> next_test_uid(1);
    inline test_uid make_test_uid() { return next_test_uid++; }
  }

} // namespace mettle

#endif
