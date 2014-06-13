#ifndef INC_METTLE_TYPE_NAME_HPP
#define INC_METTLE_TYPE_NAME_HPP

#include <string>

namespace mettle {

#if defined __clang__
template<typename T>
std::string type_name() {
  const size_t begin = sizeof("std::string mettle::type_name() [T = ") - 1;
  const size_t end = sizeof(__PRETTY_FUNCTION__) - sizeof("]");
  return std::string(__PRETTY_FUNCTION__ + begin, __PRETTY_FUNCTION__ + end);
}
#elif defined __GNUG__
template<typename T>
std::basic_string<char> type_name() {
  const size_t begin = sizeof(
    "std::basic_string<char> mettle::type_name() [with T = "
  ) - 1;
  const size_t end = sizeof(__PRETTY_FUNCTION__) - sizeof("]");
  return std::string(__PRETTY_FUNCTION__ + begin, __PRETTY_FUNCTION__ + end);
}
#else
template<typename T>
std::string type_name() {
  return typeid(T).name();
}
#endif

} // namespace mettle

#endif
