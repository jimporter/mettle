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
      match_result operator ()(U &&actual) const {
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = initial_;

        ss << "[";
        for(auto &&i : actual) {
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
    return detail::member_impl(
      "member ", false, ensure_matcher(std::forward<T>(thing))
    );
  }

  template<typename T>
  auto each(T &&thing) {
    return detail::member_impl(
      "each ", true, ensure_matcher(std::forward<T>(thing))
    );
  }

  namespace detail {
    template<typename Matcher>
    class each_impl : public matcher_tag {
    public:
      template<typename T, typename U>
      each_impl(T begin, T end, U &&meta_matcher) {
        static_assert(is_matcher_v<decltype(meta_matcher(*begin))>,
                      "meta_matcher must be a function that returns a matcher");
        for(; begin != end; ++begin)
          matchers_.push_back(meta_matcher(*begin));
      }

      template<typename U>
      match_result operator ()(U &&actual) const {
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = true;

        using std::begin, std::end;
        auto i = begin(actual), end_i = end(actual);

        ss << "[";
        for(auto &&m : matchers_) {
          if(i == end_i) {
            good = false;
            break;
          }

          match_result result = m(*i);
          good &= result;
          ola(matcher_message(result, *i));
          ++i;
        }

        // Print any remaining expected values (if `actual` is longer than the
        // list of matchers).
        good &= (i == end_i);
        for(; i != end_i; ++i)
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

    template<typename T, typename U>
    each_impl(T begin, T end, U &&meta_matcher) -> each_impl<
      std::remove_reference_t<decltype(meta_matcher(*begin))>
    >;
  }

  template<typename T, typename U>
  inline auto each(T begin, T end, U &&meta_matcher) {
    return detail::each_impl(begin, end, std::forward<U>(meta_matcher));
  }

  template<typename T, typename U>
  inline auto each(T &thing, U &&meta_matcher) {
    using std::begin, std::end;
    return each(begin(thing), end(thing),
                std::forward<U>(meta_matcher));
  }

  template<typename T, typename U>
  inline auto each(T &&thing, U &&meta_matcher) {
    using std::begin, std::end;
    return each(std::make_move_iterator(begin(thing)),
                std::make_move_iterator(end(thing)),
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
      match_result operator ()(U &&actual) const {
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = true;

        using std::begin, std::end;
        auto i = begin(actual), end_i = end(actual);

        ss << "[";
        tuple_for_each(matchers_, [&i, &end_i, &good, &ola](const auto &m) {
          if(i == end_i) {
            good = false;
            return true;
          }

          auto result = m(*i);
          good &= result;
          ola(matcher_message(result, *i));
          ++i;
          return false;
        });

        // Print any remaining expected values (if `actual` is longer than the
        // list of matchers).
        good &= (i == end_i);
        for(; i != end_i; ++i)
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
    // No CTAD here to avoid deducing from the copy constructor.
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
      match_result operator ()(U &&actual) const {
        using UVal = std::remove_reference_t<U>;
        static_assert(std::tuple_size_v<UVal> == sizeof...(T),
                      "tuple sizes mismatch");
        std::ostringstream ss;
        ostream_list_append ola(ss);
        bool good = true;

        ss << "[";
        static_for<sizeof...(T)>([this, &actual, &good, &ola](auto i) {
          using std::get;
          auto v = get<i>(actual);
          auto result = get<i>(matchers_)(v);
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
      std::tuple<T...> matchers_;
    };
  }

  template<typename ...T>
  inline auto tuple(T &&...things) {
    // No CTAD here to avoid deducing from the copy constructor.
    return detail::tuple_impl<ensure_matcher_t<T>...>(
      ensure_matcher(std::forward<T>(things))...
    );
  }

  inline auto sorted() {
    return basic_matcher([](auto &&actual) {
      using std::begin, std::end;
      return std::is_sorted(begin(actual), end(actual));
    }, "sorted");
  }

  template<typename T>
  auto sorted(T &&compare) {
    return basic_matcher(
      std::forward<T>(compare),
      [](auto &&actual, auto &&compare) {
        using std::begin, std::end;
        return std::is_sorted(begin(actual), end(actual), compare);
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
      bool operator ()(U &&actual) const {
        using std::begin, std::end;
        return std::is_permutation(
          begin(actual), end(actual),
          begin(container_.value), end(container_.value)
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
      bool operator ()(U &&actual) const {
        using std::begin, std::end;
        return std::is_permutation(
          begin(actual), end(actual),
          begin(container_.value), end(container_.value),
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
    return detail::permutation_impl(std::forward<T>(container));
  }

  template<typename T, typename Pred>
  inline auto permutation(T &&container, Pred &&predicate) {
    return detail::permutation_pred_impl(
      std::forward<T>(container), std::forward<Pred>(predicate)
    );
  }

  template<typename T>
  inline auto permutation(T begin, T end) {
    return permutation(detail::range(std::move(begin), std::move(end)));
  }

  template<typename T, typename Pred>
  inline auto permutation(T begin, T end, Pred &&predicate) {
    return permutation(detail::range(std::move(begin), std::move(end)),
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
