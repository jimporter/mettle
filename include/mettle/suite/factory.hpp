#ifndef INC_METTLE_SUITE_FACTORY_HPP
#define INC_METTLE_SUITE_FACTORY_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "../detail/any_capture.hpp"

namespace mettle {

  namespace detail {

    struct auto_factory_t {
      template<typename T>
      T make() const {
        return {};
      }
    };

    struct type_only_factory_t {
      template<typename T>
      void make() const {}
    };

    template<typename ...Args>
    class bind_factory_t {
      using tuple_type = std::tuple<detail::any_capture<Args>...>;
    public:
      template<typename ...CallArgs>
      explicit bind_factory_t(CallArgs &&...args)
        requires std::constructible_from<tuple_type, CallArgs...> :
        args_(args...) {}

      template<typename T>
      T make() const {
        using Indices = std::make_index_sequence<std::tuple_size_v<tuple_type>>;
        return make_impl<T>(Indices());
      }
    private:
      template<typename T, std::size_t ...I>
      T make_impl(std::index_sequence<I...>) const {
        return T(detail::unwrap_capture(std::get<I>(args_))...);
      }

      tuple_type args_;
    };

  }

  inline detail::auto_factory_t auto_factory;
  inline detail::type_only_factory_t type_only;

  template<typename ...Args>
  auto bind_factory(Args &&...args) {
    return detail::bind_factory_t<std::remove_reference_t<Args>...>(
      std::forward<Args>(args)...
    );
  }

} // namespace mettle

#endif
