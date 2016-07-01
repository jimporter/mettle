#ifndef INC_METTLE_SUITE_FACTORY_HPP
#define INC_METTLE_SUITE_FACTORY_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace mettle {

namespace detail {

  struct auto_factory_t {
    constexpr auto_factory_t() {}

    template<typename T>
    T make() const {
      return {};
    }
  };

  struct type_only_factory_t {
    constexpr type_only_factory_t() {}

    template<typename T>
    void make() const {}
  };

  template<typename ...Args>
  class bind_factory_t {
    using tuple_type = std::tuple<Args...>;
  public:
    template<typename ...CallArgs, typename = std::enable_if_t<
      std::is_constructible<tuple_type, CallArgs...>::value
    >> explicit bind_factory_t(CallArgs &&...args) : args_(args...) {}

    template<typename T>
    T make() const {
      using Indices = std::make_index_sequence<
        std::tuple_size<tuple_type>::value
      >;
      return make_impl<T>(Indices());
    }
  private:
    template<typename T, std::size_t ...I>
    T make_impl(std::index_sequence<I...>) const {
      return T(std::get<I>(args_)...);
    }

    tuple_type args_;
  };

}

constexpr detail::auto_factory_t auto_factory;
constexpr detail::type_only_factory_t type_only;

// XXX: Work around GCC bug 65308 and don't return auto here.
template<typename ...Args>
detail::bind_factory_t<typename std::remove_reference<Args>::type...>
bind_factory(Args &&...args) {
  return detail::bind_factory_t<typename std::remove_reference<Args>::type...>(
    std::forward<Args>(args)...
  );
}

} // namespace mettle

#endif
