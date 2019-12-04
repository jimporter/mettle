#ifndef INC_METTLE_MATCHERS_COLLECTION_HPP
#define INC_METTLE_MATCHERS_COLLECTION_HPP

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <tuple>
#include <vector>

#include "core.hpp"

namespace mettle {

  namespace detail {
    template<typename Matcher>
    class member_impl : public matcher_tag {
    public:
      member_impl(std::string desc, bool initial, Matcher matcher)
        : desc_(std::move(desc)), initial_(initial),
          matcher_(std::move(matcher)) {}

      template<typename U>
      match_result operator ()(const U &value) const {
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = initial_;

        ss << "[";
        for(auto &&i : value) {
          auto result = matcher_(i);
          if(result != initial_)
            good = result;
          ola(matcher_message(result, i));
        }
        ss << "]";

        return {good, ss.str()};
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
      match_result operator ()(const U &value) const {
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = true;

        ss << "[";
        auto i = std::begin(value), end = std::end(value);
        for(auto &&m : matchers_) {
          if(i == end) {
            good = false;
            break;
          }

          match_result result = m(*i);
          good &= result;
          ola(matcher_message(result, *i));
          ++i;
        }

        // Print any remaining expected values (if `value` is longer than the
        // list of matchers).
        good &= (i == end);
        for(; i != end; ++i)
          ola(to_printable(*i));

        ss << "]";

        return {good, ss.str()};
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
    static_assert(is_matcher_v<Matcher>,
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
        ostream_list_append ola(ss);
        bool good = true;

        ss << "[";
        auto i = std::begin(value), end = std::end(value);
        tuple_for_each(matchers_, [&i, &end, &good, &ola](const auto &m) {
          if(i == end) {
            good = false;
            return true;
          }

          auto result = m(*i);
          good &= result;
          ola(matcher_message(result, *i));
          ++i;
          return false;
        });

        // Print any remaining expected values (if `value` is longer than the
        // list of matchers).
        good &= (i == end);
        for(; i != end; ++i)
          ola(to_printable(*i));

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

  namespace detail {
    template<typename ...T>
    class tuple_impl : public matcher_tag {
    public:
      tuple_impl(T ...matchers) : matchers_(std::move(matchers)...) {}

      template<typename U>
      match_result operator ()(const U &value) const {
        static_assert(std::tuple_size<U>::value == sizeof...(T),
                      "tuple sizes mismatch");
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = true;

        ss << "[";
        static_for<sizeof...(T)>([this, &value, &good, &ola](auto i) {
          auto v = std::get<i>(value);
          auto result = std::get<i>(matchers_)(v);
          good &= result;
          ola(matcher_message(result, v));
          return false;
        });
        ss << "]";

        return {good, ss.str()};
      }

      std::string desc() const {
        return "[" + stringify(tuple_joined(matchers_, [](auto &&matcher) {
          return matcher.desc();
        })) + "]";
      }
    private:
      template<std::size_t I, typename U>
      auto match(const U &value) const {
        return std::get<I>(matchers_)(std::get<I>(value));
      }

      std::tuple<T...> matchers_;
    };
  }

  template<typename ...T>
  inline auto tuple(T &&...things) {
    return detail::tuple_impl<ensure_matcher_t<T>...>(
      ensure_matcher(std::forward<T>(things))...
    );
  }

  inline auto sorted() {
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

  namespace detail {
    template<typename T>
    class range {
    public:
      range(T begin, T end) : begin_(begin), end_(end) {}
      T begin() const { return begin_; }
      T end() const { return end_; }
    private:
      T begin_, end_;
    };

    template<typename T>
    class permutation_impl : public matcher_tag {
    public:
      permutation_impl(T container) : container_(std::move(container)) {}

      template<typename U>
      bool operator ()(const U &actual) const {
        return std::is_permutation(
          std::begin(actual), std::end(actual),
          std::begin(container_.value), std::end(container_.value)
        );
      }

      std::string desc() const {
        std::ostringstream ss;
        ss << "permutation of " << to_printable(container_.value);
        return ss.str();
      }
    private:
      any_capture<T> container_;
    };

    template<typename T, typename Predicate>
    class permutation_pred_impl : public matcher_tag {
    public:
      permutation_pred_impl(T container, Predicate predicate)
        : container_(std::move(container)), predicate_(std::move(predicate)) {}

      template<typename U>
      bool operator ()(const U &actual) const {
        return std::is_permutation(
          std::begin(actual), std::end(actual),
          std::begin(container_.value), std::end(container_.value),
          predicate_
        );
      }

      std::string desc() const {
        std::ostringstream ss;
        ss << "permutation of " << to_printable(container_.value) << " for "
           << to_printable(predicate_);
        return ss.str();
      }
    private:
      any_capture<T> container_;
      Predicate predicate_;
    };
  }

  template<typename T>
  inline auto permutation(T &&container) {
    using Value = typename std::remove_reference<T>::type;
    return detail::permutation_impl<Value>(std::forward<T>(container));
  }

  template<typename T, typename Pred>
  inline auto permutation(T &&container, Pred &&predicate) {
    using Value = typename std::remove_reference<T>::type;
    using PredValue = typename std::remove_reference<Pred>::type;
    return detail::permutation_pred_impl<Value, PredValue>(
      std::forward<T>(container), std::forward<Pred>(predicate)
    );
  }

  template<typename T>
  inline auto permutation(T begin, T end) {
    return permutation(detail::range<T>(begin, end));
  }

  template<typename T, typename Pred>
  inline auto permutation(T begin, T end, Pred &&predicate) {
    return permutation(detail::range<T>(begin, end),
                       std::forward<Pred>(predicate));
  }

  template<typename T>
  inline auto permutation(std::initializer_list<T> list) {
    return permutation(std::vector<T>{list});
  }

  template<typename T, typename Pred>
  inline auto permutation(std::initializer_list<T> list, Pred &&predicate) {
    return permutation(std::vector<T>{list}, std::forward<Pred>(predicate));
  }

} // namespace mettle

#endif
