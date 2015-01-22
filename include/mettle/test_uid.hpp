#ifndef INC_METTLE_TEST_UID_HPP
#define INC_METTLE_TEST_UID_HPP

#include <atomic>
#include <cstdint>

namespace mettle {

using test_uid = std::uint64_t;

namespace detail {
  inline test_uid make_test_uid() {
    static std::atomic<test_uid> id_(0);
    return id_++;
  }
}

} // namespace mettle

#endif
