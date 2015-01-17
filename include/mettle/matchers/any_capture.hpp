#ifndef INC_METTLE_MATCHERS_ANY_CAPTURE_HPP
#define INC_METTLE_MATCHERS_ANY_CAPTURE_HPP

#include <utility>

namespace mettle {

template<typename T>
class any_capture {
public:
  constexpr any_capture(const T &t) : value(t) {}
  constexpr any_capture(T &&t) : value(std::move(t)) {}

  T value;
};

template<typename T, size_t N>
class any_capture<T[N]> {
public:
  constexpr any_capture(const T (&t)[N])
    : any_capture(t, std::make_index_sequence<N>()) {}
  constexpr any_capture(T (&&t)[N])
    : any_capture(std::move(t), std::make_index_sequence<N>()) {}

  T value[N];
private:
  template<size_t ...I>
  constexpr any_capture(const T (&t)[N], std::index_sequence<I...>)
    : value{t[I]...} {}

  template<size_t ...I>
  constexpr any_capture(T (&&t)[N], std::index_sequence<I...>)
    : value{std::move(t[I])...} {}
};

template<typename T>
class any_capture<T[]> {
  static_assert(std::is_same<T, void>::value,
                "any_capture can't be used by incomplete array types");
};

} // namespace mettle

#endif
