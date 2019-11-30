#ifndef INC_METTLE_DRIVER_LOG_XML_WRITER_HPP
#define INC_METTLE_DRIVER_LOG_XML_WRITER_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

namespace mettle::log::xml {

  bool METTLE_PUBLIC valid_name(const std::string &);

  class METTLE_PUBLIC node {
  public:
    virtual ~node() {}
    virtual void write(indenting_ostream &) const = 0;
  };
  using node_ptr = std::unique_ptr<node>;

  class METTLE_PUBLIC element : public node {
  public:
    element(std::string t) : tag_(std::move(t)) {
      assert(valid_name(tag_));
    }

    element(element &&) = default;

    static auto make(std::string t) {
      return std::make_unique<element>(std::move(t));
    }

    void write(indenting_ostream &out) const override;

    void attr(std::string name, std::string value) {
      assert(valid_name(name));
      attrs_[std::move(name)] = std::move(value);
    }

    std::size_t children_size() const {
      return children_.size();
    }

    void append_child(node_ptr n) {
      children_.push_back(std::move(n));
    }
  private:
    std::string tag_;
    std::map<std::string, std::string> attrs_;
    std::vector<node_ptr> children_;
  };
  using element_ptr = std::unique_ptr<element>;

  class METTLE_PUBLIC text : public node {
  public:
    text(std::string t) : text_content_(std::move(t)) {}

    static auto make(std::string t) {
      return std::make_unique<text>(std::move(t));
    }

    void write(indenting_ostream &out) const override;
  private:
    std::string text_content_;
  };
  using text_ptr = std::unique_ptr<text>;

  class METTLE_PUBLIC document {
  public:
    document(std::string name) : root_(std::move(name)) {}

    inline void write(std::ostream &out) const {
      indenting_ostream iout(out);
      iout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      root_.write(iout);
    }

    inline element * root() {
      return &root_;
    }
  private:
    element root_;
  };

} // namespace mettle::log::xml

#endif
