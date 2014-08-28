#ifndef INC_METTLE_FILTERS_HPP
#define INC_METTLE_FILTERS_HPP

#include <vector>

#include <mettle/attributes.hpp>

namespace mettle {

class attr_filter {
public:
  struct filter_item {
    std::string attribute;
    std::function<bool(const attr_instance *)> func;
  };

  attr_filter() = default;
  attr_filter(const std::initializer_list<filter_item> &i) : filters_(i) {}

  filter_result operator ()(const attr_list &attrs) const;

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
    return attr && attr->value.count(value);
  }};
}

class attr_filter_set {
public:
  attr_filter_set() = default;
  attr_filter_set(const std::initializer_list<attr_filter> &i) : filters_(i) {}

  filter_result operator ()(const attr_list &attrs) const;

  void insert(const attr_filter &item) {
    filters_.push_back(item);
  }

  void insert(attr_filter &&item) {
    filters_.push_back(std::move(item));
  }
private:
  std::vector<attr_filter> filters_;
};

} // namespace mettle

#endif
