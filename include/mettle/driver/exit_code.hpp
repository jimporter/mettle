#ifndef INC_METTLE_DRIVER_EXIT_CODE_HPP
#define INC_METTLE_DRIVER_EXIT_CODE_HPP

namespace mettle::exit_code {

  // Exit codes >= 64 are designed to match sysexits.h values.
  constexpr const int success = 0;
  constexpr const int failure = 1;
  constexpr const int timeout = 32;
  constexpr const int bad_args = 64;
  constexpr const int no_inputs = 66;
  constexpr const int unknown_error = 70;
  constexpr const int fatal = 71;

} // namespace mettle::exit_code

#endif
