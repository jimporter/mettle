#ifndef INC_METTLE_DETAIL_ANY_CAPTURE_HPP
#define INC_METTLE_DETAIL_ANY_CAPTURE_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

namespace mettle::detail {

  struct any_capture_tag {};

  template<typename T, typename Enable = void>
  class capture_array;

  template<typename T, std::size_t N>
  class capture_array<T[N], std::enable_if_t<std::is_trivial_v<T>>>
    : public any_capture_tag {
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
  class capture_array<T[N], std::enable_if_t<!std::is_trivial_v<T>>>
    : public any_capture_tag {
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
    static_assert(std::is_same<T, void>::value,
                  "any_capture can't be used by incomplete array types");
  };

  template<typename T, typename = void>
  struct any_capture_type {
    using type = T;
  };
  template<typename T>
  struct any_capture_type<T, std::void_t<typename capture_array<T>::type>> {
    using type = capture_array<T>;
  };
  template<typename Ret, typename ...Args>
  struct any_capture_type<Ret(Args...)> {
    using type = Ret(*)(Args...);
  };

  template<typename T>
  using any_capture = typename any_capture_type<T>::type;

  template<typename T>
  decltype(auto) unwrap_capture(T &&t) {
    using V = std::remove_reference_t<std::remove_cv_t<T>>;
    if constexpr(std::is_base_of_v<any_capture_tag, V>)
      return (t.value);
    else
      return std::forward<T>(t);
  }

} // namespace mettle::detail

#endif
