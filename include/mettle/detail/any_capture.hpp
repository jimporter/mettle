#ifndef INC_METTLE_DETAIL_ANY_CAPTURE_HPP
#define INC_METTLE_DETAIL_ANY_CAPTURE_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

#include "forward_like.hpp"

namespace mettle::detail {

  struct any_capture_tag {};

  template<typename T>
  class capture_array;

  template<typename T, std::size_t N> requires(std::is_trivial_v<T>)
  class capture_array<T[N]> : public any_capture_tag {
  public:
    using type = T[N];

    constexpr capture_array(const T (&t)[N]) {
      for(std::size_t i = 0; i != N; i++)
        value[i] = t[i];
    }
    constexpr capture_array(T (&&t)[N]) {
      for(std::size_t i = 0; i != N; i++)
        value[i] = std::move(t[i]);
    }

    std::remove_const_t<T> value[N];
  };

// Ignore warnings from MSVC about overly-long decorated names.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4503)
#endif

  template<typename T, std::size_t N>
  class capture_array<T[N]> : public any_capture_tag {
  public:
    using type = T[N];

    constexpr capture_array(const T (&t)[N])
      : capture_array(t, std::make_index_sequence<N>()) {}
    constexpr capture_array(T (&&t)[N])
      : capture_array(std::move(t), std::make_index_sequence<N>()) {}

    T value[N];
  private:
    template<std::size_t ...I>
    constexpr capture_array(const T (&t)[N], std::index_sequence<I...>)
      : value{t[I]...} {}

    template<std::size_t ...I>
    constexpr capture_array(T (&&t)[N], std::index_sequence<I...>)
      : value{std::move(t[I])...} {}
  };

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

  template<typename T>
  class capture_array<T[]> : public any_capture_tag {
    static_assert(std::is_same_v<T, void>,
                  "any_capture can't be used by incomplete array types");
  };

  template<typename T, typename = void>
  struct any_capture_type { using type = T; };
  template<typename T> requires(std::is_array_v<T>)
  struct any_capture_type<T> { using type = capture_array<T>; };
  template<typename Ret, typename ...Args>
  struct any_capture_type<Ret(Args...)> { using type = Ret(*)(Args...); };

  template<typename T>
  using any_capture = typename any_capture_type<T>::type;

  template<typename T>
  inline auto && unwrap_capture(T &&t) {
    using V = std::remove_reference_t<std::remove_cv_t<T>>;
    if constexpr(std::is_base_of_v<any_capture_tag, V>)
      return detail::forward_like<T>(t.value);
    else
      return std::forward<T>(t);
  }

} // namespace mettle::detail

#endif
