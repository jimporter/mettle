#ifndef INC_METTLE_MATCHERS_COMBINATORIC_HPP
#define INC_METTLE_MATCHERS_COMBINATORIC_HPP

#include <functional>
#include <tuple>

#include "core.hpp"
#include "../detail/tuple_algorithm.hpp"

namespace mettle {

  namespace detail {
    template<typename Filter, typename Done, typename ...T>
    class reduce_impl : public matcher_tag {
    public:
      reduce_impl(std::string desc, Filter filter, Done done,
                  T ...matchers)
        : desc_(std::move(desc)), filter_(std::move(filter)),
          done_(std::move(done)), matchers_(std::move(matchers)...) {}

      template<typename U>
      match_result operator ()(U &&actual) const {
        match_result result = done_(false);
        tuple_for_each(matchers_, [&, this](const auto &matcher) {
          auto m = filter_(matcher(actual));
          bool done = done_(m);
          if(done)
            result = std::move(m);
          return done;
        });
        return result;
      }

      std::string desc() const {
        std::ostringstream ss;
        ss << desc_ << "(" << tuple_joined(matchers_, [](auto &&matcher) {
          return matcher.desc();
        }) << ")";
        return ss.str();
      }
    private:
      const std::string desc_;
      Filter filter_;
      Done done_;
      std::tuple<T...> matchers_;
    };
  }

  // We'd call these any_of, all_of, and none_of like the std:: functions, but
  // doing so would introduce an ambiguity where the std:: versions could be
  // preferred via ADL if we pass in three arguments.

  template<typename ...T>
  inline auto any(T &&...things) {
    return detail::reduce_impl("any of", std::identity{}, std::identity{},
                               ensure_matcher(std::forward<T>(things))...);
  }

  template<typename ...T>
  inline auto all(T &&...things) {
    return detail::reduce_impl("all of", std::identity{}, std::logical_not<>{},
                               ensure_matcher(std::forward<T>(things))...);
  }

  template<typename ...T>
  inline auto none(T &&...things) {
    return detail::reduce_impl(
      "none of", std::logical_not<>{}, std::logical_not<>{},
      ensure_matcher(std::forward<T>(things))...
    );
  }

} // namespace mettle

#endif
