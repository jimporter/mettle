#ifndef INC_METTLE_MATCHERS_RELATIONAL_HPP
#define INC_METTLE_MATCHERS_RELATIONAL_HPP

#include "core.hpp"

namespace mettle {

  // Note: equal_to is declared in core.hpp, since it's pretty important!

  template<typename T>
  inline auto not_equal_to(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::not_equal_to<>(),
                         "not ");
  }

  template<typename T>
  inline auto greater(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::greater<>(), "> ");
  }

  template<typename T>
  inline auto greater_equal(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::greater_equal<>(),
                         ">= ");
  }

  template<typename T>
  inline auto less(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::less<>(), "< ");
  }

  template<typename T>
  inline auto less_equal(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::less_equal<>(), "<= ");
  }

  enum class interval {
    closed     = 0b00,
    left_open  = 0b10,
    right_open = 0b01,
    open       = 0b11
  };

  template<typename Low, typename High>
  class in_interval : public matcher_tag {
  public:
    in_interval(detail::any_capture<Low> low, detail::any_capture<High> high,
                interval bounds = interval::right_open) :
      low_(std::move(low)), high_(std::move(high)), bounds_(bounds) {}

    template<typename U>
    bool operator ()(U &&actual) const {
      if (left_open()) {
        if (actual <= low_.value) return false;
      } else {
        if (actual < low_.value) return false;
      }

      if (right_open()) {
        if (actual >= high_.value) return false;
      } else {
        if (actual > high_.value) return false;
      }

      return true;
    }

    std::string desc() const {
      std::ostringstream ss;
      ss << "in " << (left_open() ? "(" : "[") << to_printable(low_.value)
         << " .. " << to_printable(high_.value) << (right_open() ? ")" : "]");
      return ss.str();
    }
  private:
    inline bool left_open() const {
      return static_cast<int>(bounds_) & static_cast<int>(interval::left_open);
    }

    inline bool right_open() const {
      return static_cast<int>(bounds_) & static_cast<int>(interval::right_open);
    }

    detail::any_capture<Low> low_;
    detail::any_capture<High> high_;
    interval bounds_;
  };

  template<typename Low, typename High>
  in_interval(Low &&, High &&) -> in_interval<
    std::remove_reference_t<Low>, std::remove_reference_t<High>
  >;
  template<typename Low, typename High>
  in_interval(Low &&, High &&, interval) -> in_interval<
    std::remove_reference_t<Low>, std::remove_reference_t<High>
  >;

} // namespace mettle

#endif
