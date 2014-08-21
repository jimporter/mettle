#ifndef INC_METTLE_ATTRIBUTES_HPP
#define INC_METTLE_ATTRIBUTES_HPP

#include <set>
#include <string>

namespace mettle {

class attribute;

class attr_instance {
public:
  explicit attr_instance(const attribute &attr, const std::string &comment = "")
    : attr_(attr), comment_(comment) {}

  std::string name() const;
  bool skip() const;

  const std::string & comment() const {
    return comment_;
  }
private:
  const attribute &attr_;
  std::string comment_;
};

inline bool operator <(const attr_instance &lhs, const attr_instance &rhs) {
  return lhs.name() < rhs.name();
}

class attribute {
public:
  constexpr attribute(const char *name, bool skip = false)
    : name_(name), skip_(skip) {}

  operator const attr_instance() const {
    return attr_instance(*this);
  }

  const attr_instance operator ()(const std::string &comment) const {
    return attr_instance(*this, comment);
  }

  std::string name() const {
    return name_;
  }

  bool skip() const {
    return skip_;
  }
private:
  const char *name_;
  bool skip_;
};

inline std::string attr_instance::name() const {
  return attr_.name();
}

inline bool attr_instance::skip() const {
  return attr_.skip();
}

using attribute_list = std::set<attr_instance>;

inline attribute_list
unite(const attribute_list &lhs, const attribute_list &rhs) {
  attribute_list all_attrs;
  std::set_union(
    lhs.begin(), lhs.end(),
    rhs.begin(), rhs.end(),
    std::inserter(all_attrs, all_attrs.begin())
  );
  return all_attrs;
}

constexpr attribute skip("skip", true);

} // namespace mettle

#endif
