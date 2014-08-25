#ifndef INC_METTLE_ATTRIBUTES_HPP
#define INC_METTLE_ATTRIBUTES_HPP

#include <set>
#include <stdexcept>
#include <string>

namespace mettle {

enum class attr_action {
  run,
  skip,
  hide
};

class attr_base {
protected:
  constexpr attr_base(const char *name, attr_action action = attr_action::run)
    : name_(name), action_(action) {
    if(action == attr_action::hide)
      throw std::invalid_argument("attr's action can't be \"hide\"");
  }
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
  using value_type = std::set<std::string>;
  explicit attr_instance(const attr_base &attr, const value_type &value)
    : attr_(attr), value_(value) {}

  std::string name() const {
    return attr_.name();
  }

  attr_action action() const {
    return attr_.action();
  }

  bool empty() const {
    return value_.empty();
  }

  const value_type & value() const {
    return value_;
  }
private:
  const attr_base &attr_;
  value_type value_;
};

namespace detail {
  struct attr_less {
    using is_transparent = void;

    bool operator ()(const attr_instance &lhs, const attr_instance &rhs) const {
      return lhs.name() < rhs.name();
    }

    bool operator ()(const attr_instance &lhs, const std::string &rhs) const {
      return lhs.name() < rhs;
    }

    bool operator ()(const std::string &lhs, const attr_instance &rhs) const {
      return lhs < rhs.name();
    }
  };
}

class bool_attr : public attr_base {
public:
  constexpr bool_attr(const char *name, attr_action action = attr_action::run)
    : attr_base(name, action) {}

  operator const attr_instance() const {
    return attr_instance(*this, {});
  }

  template<typename T>
  const attr_instance operator ()(T &&comment) const {
    return attr_instance(*this, {std::forward<T>(comment)});
  }
};

class string_attr : public attr_base {
public:
  constexpr string_attr(const char *name)
    : attr_base(name) {}

  template<typename T>
  const attr_instance operator ()(T &&value) const {
    return attr_instance(*this, {std::forward<T>(value)});
  }
};

class list_attr : public attr_base {
public:
  constexpr list_attr(const char *name)
    : attr_base(name) {}

  template<typename ...T>
  const attr_instance operator ()(T &&...args) const {
    return attr_instance(*this, {std::forward<T>(args)...});
  }
};

using attr_list = std::set<attr_instance, detail::attr_less>;

inline attr_list unite(const attr_list &lhs, const attr_list &rhs) {
  // XXX: Make list_attrs merge instead of overriding.
  attr_list all_attrs;
  std::set_union(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
    std::inserter(all_attrs, all_attrs.begin()), detail::attr_less()
  );
  return all_attrs;
}

// XXX: Pull these into a .cpp file?
class attr_filter {
public:
  struct filter_item {
    std::string attr;
    std::function<bool(const attr_instance *)> f;
  };

  attr_filter() = default;
  attr_filter(const std::initializer_list<filter_item> &i) : filters_(i) {}

  std::pair<attr_action, const attr_instance*>
  operator ()(const attr_list &attrs) const {
    attr_list explicitly_shown;
    for(const auto &f : filters_) {
      auto i = attrs.find(f.attr);
      const attr_instance *attr = i == attrs.end() ? nullptr: &*i;
      if(!f.f(attr))
        return {attr_action::hide, attr};
      else if(attr)
        explicitly_shown.insert(*attr);
    }
    for(const auto &attr : attrs) {
      if(attr.action() == attr_action::skip && !explicitly_shown.count(attr))
        return {attr_action::skip, &attr};
    }
    return {attr_action::run, nullptr};
  }

  void insert(const filter_item &item) {
    filters_.push_back(item);
  }

  void insert(filter_item &&item) {
    filters_.push_back(std::move(item));
  }
private:
  std::vector<filter_item> filters_;
};

inline attr_filter::filter_item
has_attr(const std::string &name) {
  return {name, [](const attr_instance *attr) {
    return attr;
  }};
}

inline attr_filter::filter_item
has_attr(const std::string &name, const std::string &value) {
  return {name, [value](const attr_instance *attr) {
    return attr && attr->value().count(value);
  }};
}

class attr_filter_set {
public:
  attr_filter_set() = default;
  attr_filter_set(const std::initializer_list<attr_filter> &i) : filters_(i) {}

  std::pair<attr_action, const attr_instance*>
  operator ()(const attr_list &attrs) const {
    // Pretend we always have a default filter, if nothing else.
    if(filters_.empty())
      return attr_filter{}(attrs);

    bool set = false;
    std::pair<attr_action, const attr_instance*> result;
    for(const auto &f : filters_) {
      auto curr = f(attrs);
      switch(curr.first) {
      case attr_action::run:
        return curr;
      case attr_action::skip:
        if(!set || result.first == attr_action::hide) {
          result = curr;
          set = true;
        }
        break;
      case attr_action::hide:
        if(!set) {
          result = curr;
          set = true;
        }
        break;
      }
    }
    return result;
  }

  void insert(const attr_filter &item) {
    filters_.push_back(item);
  }

  void insert(attr_filter &&item) {
    filters_.push_back(std::move(item));
  }
private:
  std::vector<attr_filter> filters_;
};

constexpr bool_attr skip("skip", attr_action::skip);

} // namespace mettle

#endif
