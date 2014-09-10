#ifndef INC_METTLE_FILTERS_HPP
#define INC_METTLE_FILTERS_HPP

#include <vector>

#include <mettle/filters_core.hpp>

namespace mettle {

struct attr_filter_item {
  std::string attribute;
  std::function<bool(const attr_instance *)> func;
};

inline attr_filter_item
has_attr(const std::string &name) {
  return {name, [](const attr_instance *attr) -> bool {
    return attr;
  }};
}

inline attr_filter_item
has_attr(const std::string &name, const std::string &value) {
  return {name, [value](const attr_instance *attr) -> bool {
    return attr && attr->value.count(value);
  }};
}

inline attr_filter_item
operator !(const attr_filter_item &filter) {
  auto f = [func = filter.func](const attr_instance *attr) -> bool {
    return !func(attr);
  };
  return {filter.attribute, std::move(f)};
}

inline attr_filter_item
operator !(attr_filter_item &&filter) {
  auto f = [func = std::move(filter.func)](const attr_instance *attr) -> bool {
    return !func(attr);
  };
  return {std::move(filter.attribute), std::move(f)};
}

class attr_filter {
public:
  using value_type = attr_filter_item;
  using container_type = std::vector<attr_filter_item>;
  using iterator = container_type::const_iterator;

  attr_filter() = default;
  attr_filter(const std::initializer_list<value_type> &i) : filters_(i) {}

  filter_result operator ()(const attributes &attrs) const;

  void insert(const value_type &item) {
    filters_.push_back(item);
  }

  void insert(value_type &&item) {
    filters_.push_back(std::move(item));
  }

  bool empty() const {
    return filters_.empty();
  }

  size_t size() const {
    return filters_.size();
  }

  iterator begin() const {
    return filters_.begin();
  }

  iterator end() const {
    return filters_.end();
  }
private:
  container_type filters_;
};

class attr_filter_set {
public:
  using value_type = attr_filter;
  using container_type = std::vector<value_type>;
  using iterator = container_type::const_iterator;

  attr_filter_set() = default;
  attr_filter_set(const std::initializer_list<value_type> &i) : filters_(i) {}

  filter_result operator ()(const attributes &attrs) const;

  void insert(const value_type &item) {
    filters_.push_back(item);
  }

  void insert(value_type &&item) {
    filters_.push_back(std::move(item));
  }

  bool empty() const {
    return filters_.empty();
  }

  size_t size() const {
    return filters_.size();
  }

  iterator begin() const {
    return filters_.begin();
  }

  iterator end() const {
    return filters_.end();
  }
private:
  container_type filters_;
};

} // namespace mettle

#endif
