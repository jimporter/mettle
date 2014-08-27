#include "filters.hpp"

namespace mettle {
  filter_result attr_filter::operator ()(const attr_list &attrs) const {
    attr_list explicitly_shown;
    for(const auto &f : filters_) {
      auto i = attrs.find(f.attr);
      const attr_instance *attr = i == attrs.end() ? nullptr: &*i;
      if(!f.func(attr))
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

  filter_result attr_filter_set::operator ()(const attr_list &attrs) const {
    // Pretend we always have a default filter, if nothing else.
    if(filters_.empty())
      return default_attr_filter{}(attrs);

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
}
