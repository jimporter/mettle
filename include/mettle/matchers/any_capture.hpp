#ifndef INC_METTLE_MATCHERS_ANY_CAPTURE_HPP
#define INC_METTLE_MATCHERS_ANY_CAPTURE_HPP

#include <cstdint>
#include <utility>

// XXX: Remove this when MSVC supports "optional" constexpr on templates.
#if !defined(_MSC_VER) || defined(__clang__)
#  define METTLE_CONSTEXPR constexpr
#else
#  define METTLE_CONSTEXPR
#endif

namespace mettle {

template<typename T>
class any_capture {
public:
  using type = T;

  METTLE_CONSTEXPR any_capture(const T &t) : value(t) {}
  METTLE_CONSTEXPR any_capture(T &&t) : value(std::move(t)) {}

  T value;
};

template<typename T, std::size_t N>
class any_capture<T[N]> {
public:
  using type = T[N];

  METTLE_CONSTEXPR any_capture(const T (&t)[N])
    : any_capture(t, std::make_index_sequence<N>()) {}
  METTLE_CONSTEXPR any_capture(T (&&t)[N])
    : any_capture(std::move(t), std::make_index_sequence<N>()) {}

  T value[N];
private:
  template<std::size_t ...I>
  METTLE_CONSTEXPR any_capture(const T (&t)[N], std::index_sequence<I...>)
    : value{t[I]...} {}

  template<std::size_t ...I>
  METTLE_CONSTEXPR any_capture(T (&&t)[N], std::index_sequence<I...>)
    : value{std::move(t[I])...} {}
};

template<typename Ret, typename ...Args>
class any_capture<Ret(Args...)> : public any_capture<Ret(*)(Args...)> {
  using any_capture<Ret(*)(Args...)>::any_capture;
};

template<typename T>
class any_capture<T[]> {
  static_assert(std::is_same<T, void>::value,
                "any_capture can't be used by incomplete array types");
};

} // namespace mettle

#endif
