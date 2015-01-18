#ifndef INC_METTLE_OUTPUT_TYPE_NAME_HPP
#define INC_METTLE_OUTPUT_TYPE_NAME_HPP

#include <string>
#include <typeinfo>

namespace mettle {

#if defined(__clang__)
template<typename T>
std::string type_name() {
  const size_t begin = sizeof("std::string mettle::type_name() [T = ") - 1;
  const size_t end = sizeof(__PRETTY_FUNCTION__) - sizeof("]");
  return std::string(__PRETTY_FUNCTION__ + begin, __PRETTY_FUNCTION__ + end);
}
#elif defined(__GNUG__)
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

template<typename T>
std::string type_name(const T &t);

} // namespace mettle

#if defined(__clang__) || defined(__GNUG__)
#include <cxxabi.h>
template<typename T>
std::string mettle::type_name(const T &t) {
  int status;
  const char *mangled = typeid(t).name();
  std::unique_ptr<char, void (*)(void*)> demangled(
    abi::__cxa_demangle(mangled, nullptr, nullptr, &status),
    std::free
  );
  return status ? mangled : demangled.get();
}
#else
template<typename T>
std::string mettle::type_name(const T &t) {
  return typeid(t).name();
}
#endif

#endif
