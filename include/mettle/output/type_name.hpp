#ifndef INC_METTLE_OUTPUT_TYPE_NAME_HPP
#define INC_METTLE_OUTPUT_TYPE_NAME_HPP

#include <memory>
#include <string>
#include <typeinfo>

namespace mettle {

#if defined(__clang__)

template<typename T>
std::string type_name() {
  const auto begin = sizeof("std::string mettle::type_name() [T = ") - 1;
  const auto end = sizeof(__PRETTY_FUNCTION__) - sizeof("]");
  return std::string(__PRETTY_FUNCTION__ + begin, __PRETTY_FUNCTION__ + end);
}

#elif defined(__GNUG__)

template<typename T>
std::basic_string<char> type_name() {
  const auto begin = sizeof(
#if defined(_GLIBCXX_USE_CXX11_ABI) && _GLIBCXX_USE_CXX11_ABI == 1
    "std::__cxx11::basic_string<char> mettle::type_name() [with T = "
#else
    "std::basic_string<char> mettle::type_name() [with T = "
#endif
  ) - 1;
  const auto end = sizeof(__PRETTY_FUNCTION__) - sizeof("]");
  return std::string(__PRETTY_FUNCTION__ + begin, __PRETTY_FUNCTION__ + end);
}

#elif defined(_MSC_VER)

template<typename T>
std::string type_name() {
  const char sig[] = __FUNCSIG__;
  const auto begin = sizeof(
    "class std::basic_string<char,struct std::char_traits<char>,"
    "class std::allocator<char> > __cdecl mettle::type_name<"
  ) - 1;
  const auto end = sizeof(sig) - sizeof(">(void)");
  return std::string(sig + begin, sig + end);
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
