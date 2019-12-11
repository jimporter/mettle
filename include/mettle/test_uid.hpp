#ifndef INC_METTLE_TEST_UID_HPP
#define INC_METTLE_TEST_UID_HPP

#include <atomic>
#include <cstdint>

namespace mettle {

  using test_uid = std::uint64_t;

  namespace detail {
    inline std::atomic<test_uid> next_test_uid(1);
    inline test_uid make_test_uid() { return next_test_uid++; }

    class file_uid_maker {
    public:
      test_uid make_file_uid() {
        return next_file_uid++ << 32;
      }
    private:
      test_uid next_file_uid = 0;
    };

    // This is useful for maxing out a file-only UID so that it sorts *after*
    // file-and-test UIDs.
    inline test_uid max_local_bits(test_uid id) {
      return id | 0xffffffff;
    }
  }

} // namespace mettle

#endif
