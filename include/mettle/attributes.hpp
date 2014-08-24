#ifndef INC_METTLE_ATTRIBUTES_HPP
#define INC_METTLE_ATTRIBUTES_HPP

#include <set>
#include <string>

namespace mettle {

enum class attr_action {
  run,
  skip
};

class attr_base {
protected:
  constexpr attr_base(const char *name, attr_action action = attr_action::run)
    : name_(name), action_(action) {}
  ~attr_base() = default;
public:
  std::string name() const {
    return name_;
  }

  attr_action action() const {
    return action_;
  }
private:
  const char *name_;
  attr_action action_;
};

class attr_instance {
public:
  explicit attr_instance(const attr_base &attr, const std::string &value)
    : attr_(attr), value_(value) {}

  std::string name() const {
    return attr_.name();
  }

  attr_action action() const {
    return attr_.action();
  }

  const std::string & value() const {
    return value_;
  }
private:
  const attr_base &attr_;
  std::string value_;
};

namespace detail {
  struct attr_less {
    bool operator ()(const attr_instance &lhs, const attr_instance &rhs) const {
      return lhs.name() < rhs.name();
    }
  };
}

class bool_attr : public attr_base {
public:
  constexpr bool_attr(const char *name, attr_action action = attr_action::run)
    : attr_base(name, action) {}

  operator const attr_instance() const {
    return (*this)();
  }

  const attr_instance operator ()(const std::string &comment = "") const {
    return attr_instance(*this, comment);
  }
};

class string_attr : public attr_base {
public:
  constexpr string_attr(const char *name)
    : attr_base(name) {}

  const attr_instance operator ()(const std::string &value) const {
    return attr_instance(*this, value);
  }
};

using attr_list = std::set<attr_instance, detail::attr_less>;

inline attr_list unite(const attr_list &lhs, const attr_list &rhs) {
  attr_list all_attrs;
  std::set_union(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
    std::inserter(all_attrs, all_attrs.begin()), detail::attr_less()
  );
  return all_attrs;
}

constexpr bool_attr skip("skip", attr_action::skip);

} // namespace mettle

#endif
