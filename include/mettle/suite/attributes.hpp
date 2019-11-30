#ifndef INC_METTLE_SUITE_ATTRIBUTES_HPP
#define INC_METTLE_SUITE_ATTRIBUTES_HPP

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

namespace mettle {

  enum class test_action {
    run,
    skip,
    hide,
    indeterminate
  };

  class attr_base;

  struct attr_instance {
    using value_type = std::set<std::string>;

    const attr_base &attribute;
    const value_type value;
  };

  class attr_base {
  protected:
    attr_base(std::string name, test_action action = test_action::run)
      : name_(std::move(name)), action_(action) {
      assert(action == test_action::run || action == test_action::skip);
    }
  public:
    virtual ~attr_base() = default;

    const std::string & name() const {
      return name_;
    }

    test_action action() const {
      return action_;
    }

    virtual const attr_instance
    compose(const attr_instance &lhs, const attr_instance &rhs) const {
      assert(&lhs.attribute == this && &rhs.attribute == this);
      (void)rhs;
      return lhs;
    }
  private:
    std::string name_;
    test_action action_;
  };

  namespace detail {
    struct attr_less {
      using is_transparent = void;

      bool
      operator ()(const attr_instance &lhs, const attr_instance &rhs) const {
        return lhs.attribute.name() < rhs.attribute.name();
      }

      bool
      operator ()(const attr_instance &lhs, const std::string &rhs) const {
        return lhs.attribute.name() < rhs;
      }

      bool
      operator ()(const std::string &lhs, const attr_instance &rhs) const {
        return lhs < rhs.attribute.name();
      }
    };
  }

  class bool_attr : public attr_base {
  public:
    bool_attr(std::string name, test_action action = test_action::run)
      : attr_base(std::move(name), action) {}

    operator attr_instance() const {
      return attr_instance{*this, {}};
    }

    template<typename T>
    attr_instance operator ()(T &&comment) const {
      return attr_instance{*this, {std::forward<T>(comment)}};
    }
  };

  class string_attr : public attr_base {
  public:
    string_attr(std::string name)
      : attr_base(std::move(name)) {}

    template<typename T>
    attr_instance operator ()(T &&value) const {
      return attr_instance{*this, {std::forward<T>(value)}};
    }
  };

  class list_attr : public attr_base {
  public:
    list_attr(std::string name)
      : attr_base(std::move(name)) {}

    template<typename ...T>
    attr_instance operator ()(T &&...args) const {
      return attr_instance{*this, {std::forward<T>(args)...}};
    }

    const attr_instance
    compose(const attr_instance &lhs, const attr_instance &rhs) const override {
      assert(&lhs.attribute == this && &rhs.attribute == this);
      attr_instance::value_type merged;
      std::set_union(
        lhs.value.begin(), lhs.value.end(),
        rhs.value.begin(), rhs.value.end(),
        std::inserter(merged, merged.begin())
      );
      return attr_instance{*this, std::move(merged)};
    }
  };

  using attributes = std::set<attr_instance, detail::attr_less>;

  namespace detail {
    template<typename Input1, typename Input2, typename Output,
             typename Compare, typename Merge>
    Output merge_union(Input1 first1, Input1 last1, Input2 first2, Input2 last2,
                       Output result, Compare comp, Merge merge) {
      for(; first1 != last1; ++result) {
        if(first2 == last2)
          return std::copy(first1, last1, result);

        if(comp(*first1, *first2))
          *result = *first1++;
        else if(comp(*first2, *first1))
          *result = *first2++;
        else
          *result = merge(*first1++, *first2++);
      }
      return std::copy(first2, last2, result);
    }
  }

  inline attr_instance
  unite(const attr_instance &lhs, const attr_instance &rhs) {
    if(&lhs.attribute != &rhs.attribute)
      throw std::invalid_argument("mismatched attributes");
    return lhs.attribute.compose(lhs, rhs);
  }

  inline attributes
  unite(const attributes &lhs, const attributes &rhs) {
    attributes all_attrs;
    detail::merge_union(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
      std::inserter(all_attrs, all_attrs.begin()), detail::attr_less(),
      [](const attr_instance &lhs, const attr_instance &rhs) {
        return unite(lhs, rhs);
      }
    );
    return all_attrs;
  }

  extern bool_attr skip;

} // namespace mettle

#endif
