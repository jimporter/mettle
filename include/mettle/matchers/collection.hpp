#ifndef INC_METTLE_MATCHERS_COLLECTION_HPP
#define INC_METTLE_MATCHERS_COLLECTION_HPP

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <tuple>

#include "core.hpp"
#include "../detail/move_if.hpp"

namespace mettle {

namespace detail {

  template<typename Call, typename Result, bool Enable = true>
  struct enable_if_result : std::enable_if<
    std::is_same<typename std::result_of<Call>::type, Result>::value == Enable,
    typename std::result_of<Call>::type
  > {};

  template<typename Call, typename Result, bool Disable = true>
  using disable_if_result = enable_if_result<Call, Result, !Disable>;

  template<typename Matcher>
  class member_impl : public matcher_tag {
  public:
    member_impl(std::string desc, bool initial, Matcher matcher)
      : desc_(std::move(desc)), initial_(initial),
        matcher_(std::move(matcher)) {}

    template<typename U>
    auto operator ()(const U &value) const -> typename enable_if_result<
      const Matcher & (decltype(*std::begin(value))), match_result
    >::type {
      std::ostringstream ss;
      bool good = initial_;
      ss << "[" << joined(value, [this, &good](auto &&i) {
        auto result = matcher_(i);
        if(result != initial_)
          good = result;
        return result.message;
      }) << "]";
      return {good, ss.str()};
    }

    template<typename U>
    auto operator ()(const U &value) const -> typename disable_if_result<
      const Matcher & (decltype(*std::begin(value))), match_result
    >::type {
      for(const auto &i : value) {
        auto result = matcher_(i);
        if(result != initial_)
          return result;
      }
      return initial_;
    }

    std::string desc() const {
      return desc_ + matcher_.desc();
    }
  private:
    const std::string desc_;
    const bool initial_;
    Matcher matcher_;
  };
}

template<typename T>
auto member(T &&thing) {
  return detail::member_impl<ensure_matcher_t<T>>(
    "member ", false, ensure_matcher(std::forward<T>(thing))
  );
}

template<typename T>
auto each(T &&thing) {
  return detail::member_impl<ensure_matcher_t<T>>(
    "each ", true, ensure_matcher(std::forward<T>(thing))
  );
}

namespace detail {
  template<typename Matcher>
  class each_impl : public matcher_tag {
  public:
    template<typename T, typename U>
    each_impl(T begin, T end, U &&meta_matcher) {
      for(; begin != end; ++begin)
        matchers_.push_back(meta_matcher(*begin));
    }

    template<typename U>
    auto operator ()(const U &value) const -> typename enable_if_result<
      const Matcher & (decltype(*std::begin(value))), match_result
    >::type {
      std::ostringstream ss;
      bool good = true;
      auto m = matchers_.begin(), end = matchers_.end();

      ss << "[" << joined(value, [this, &good, &m, &end](auto &&i) {
        if(m == end) {
          good = false;
          // XXX: Come up with a good way to indicate that this is an extra
          // value we don't have a matcher for.
          return stringify(to_printable(i));
        }

        match_result result = (*m++)(i);
        good &= result;
        return result.message;
      }) << "]";

      return {good && m == end, ss.str()};
    }

    template<typename U>
    auto operator ()(const U &value) const -> typename disable_if_result<
      const Matcher & (decltype(*std::begin(value))), match_result
    >::type {
      auto m = matchers_.begin(), end = matchers_.end();
      for(auto &&i : value) {
        if(m == end)
          return false;
        auto result = (*m++)(i);
        if(!result)
          return result;
      }
      return m == end;
    }

    std::string desc() const {
      return "[" + stringify(joined(matchers_, [](const auto &matcher) {
        return matcher.desc();
      })) + "]";
    }
  private:
    std::vector<Matcher> matchers_;
  };
}

template<typename T, typename U>
inline auto each(T begin, T end, U &&meta_matcher) {
  using Matcher = decltype(meta_matcher(*begin));
  static_assert(is_matcher<Matcher>::value,
                "meta_matcher must be a function that returns a matcher");
  return detail::each_impl<decltype(meta_matcher(*begin))>(
    begin, end, std::forward<U>(meta_matcher)
  );
}

template<typename T, typename U>
inline auto each(T &thing, U &&meta_matcher) {
  return each(std::begin(thing), std::end(thing),
              std::forward<U>(meta_matcher));
}

template<typename T, typename U>
inline auto each(T &&thing, U &&meta_matcher) {
  return each(std::make_move_iterator(std::begin(thing)),
              std::make_move_iterator(std::end(thing)),
              std::forward<U>(meta_matcher));
}

template<typename T, typename U>
inline auto each(std::initializer_list<T> list, U &&meta_matcher) {
  return each(list.begin(), list.end(), std::forward<U>(meta_matcher));
}

namespace detail {
  template<typename ...T>
  class array_impl : public matcher_tag {
  public:
    array_impl(T ...matchers) : matchers_(std::move(matchers)...) {}

    template<typename U>
    match_result operator ()(const U &value) const {
      std::ostringstream ss;
      ostream_list_append append(ss);
      bool good = true;

      ss << "[";
      auto i = std::begin(value), end = std::end(value);
      tuple_for_until(matchers_, [&i, &end, &good, &append](const auto &m) {
        if(i == end) {
          good = false;
          return true;
        }

        match_result result = m(*i);
        good &= result;
        append(matcher_message(result, to_printable(*i)));
        ++i;
        return false;
      });

      // Print any remaining expected values (if `value` is longer than the list
      // of matchers). XXX: Come up with a better way to indicate that these
      // values are extra?
      good &= (i == end);
      for(; i != end; ++i)
        append(to_printable(*i));

      ss << "]";

      return {good, ss.str()};
    }

    std::string desc() const {
      return "[" + stringify(tuple_joined(matchers_, [](auto &&matcher) {
        return matcher.desc();
      })) + "]";
    }
  private:
    std::tuple<T...> matchers_;
  };
}

template<typename ...T>
inline auto array(T &&...things) {
  return detail::array_impl<ensure_matcher_t<T>...>(
    ensure_matcher(std::forward<T>(things))...
  );
}

auto sorted() {
  return make_matcher([](const auto &value) {
    return std::is_sorted(std::begin(value), std::end(value));
  }, "sorted");
}

template<typename T>
auto sorted(T &&compare) {
  return make_matcher(
    std::forward<T>(compare),
    [](const auto &value, auto &&compare) {
    return std::is_sorted(std::begin(value), std::end(value), compare);
  }, "sorted by ");
}

} // namespace mettle

#endif
