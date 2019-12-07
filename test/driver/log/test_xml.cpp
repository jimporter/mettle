#include <mettle.hpp>
using namespace mettle;

#include <sstream>

#include <mettle/driver/log/xml.hpp>

#define XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

std::string make_string(const log::xml::document &doc) {
  std::ostringstream out;
  doc.write(out);
  return out.str();
}

std::string make_string(const log::xml::node &n) {
  std::ostringstream out;
  indenting_ostream iout(out);
  n.write(iout);
  return out.str();
}

auto valid_name() {
  return basic_matcher([](const auto &value) -> bool {
    return log::xml::valid_name(value);
  }, "valid name");
}

suite<> test_xml("xml writing", [](auto &_) {
  subsuite<>(_, "valid_name()", [](auto &_) {
    _.test("empty", []() {
      expect("", is_not(valid_name()));
    });

    _.test("alphanumeric", []() {
      expect("foo", valid_name());
      expect("foo-123", valid_name());
      expect("foo_123", valid_name());
      expect("_foo", valid_name());
    });

    _.test("containing invalid characters", []() {
      expect("bad tag", is_not(valid_name()));
      expect("bad\ttag", is_not(valid_name()));
      expect("bad!", is_not(valid_name()));
      expect("'bad'", is_not(valid_name()));
      expect("\"bad\"", is_not(valid_name()));
    });

    _.test("starting with numbers, hyphens, or periods", []() {
      expect("123", is_not(valid_name()));
      expect("1foo", is_not(valid_name()));
      expect(".foo", is_not(valid_name()));
      expect("-foo", is_not(valid_name()));
    });

    _.test("starting with \"xml\"", []() {
      expect("xml", is_not(valid_name()));
      expect("xmlabc", is_not(valid_name()));
      expect("XmL", is_not(valid_name()));
    });
  });

  subsuite<>(_, "element", [](auto &_) {
    _.test("empty", []() {
      log::xml::element e("element");
      expect(make_string(e), equal_to("<element/>\n"));
    });

    _.test("attributes", []() {
      log::xml::element e("element");
      e.attr("attr1", "value");
      e.attr("attr2", "<my &\n\"value\">");
      expect(make_string(e), equal_to(
        "<element attr1=\"value\" "
                 "attr2=\"&lt;my &amp;&#10;&quot;value&quot;&gt;\"/>\n"
      ));
    });

    _.test("children", []() {
      log::xml::element e("element");
      e.append_child(log::xml::element::make("child"));
      expect(make_string(e), equal_to("<element>\n  <child/>\n</element>\n"));
    });

    _.test("make", []() {
      auto e = log::xml::element::make("element");
      e->attr("attr", "value");
      expect(make_string(*e), equal_to("<element attr=\"value\"/>\n"));
    });
  });

  subsuite<>(_, "text", [](auto &_) {
    _.test("basic", []() {
      log::xml::text t("hello world");
      expect(make_string(t), equal_to("hello world\n"));
    });

    _.test("escaped", []() {
      log::xml::text t("<my &\n\"value\">");
      expect(make_string(t), equal_to("&lt;my &amp;\n&quot;value&quot;&gt;\n"));
    });

    _.test("indented", []() {
      log::xml::element e("element");
      e.append_child(log::xml::text::make("line 1\nline 2"));
      expect(make_string(e), equal_to(
        "<element>\n"
        "  line 1\n"
        "  line 2\n"
        "</element>\n"
     ));
    });

    _.test("make", []() {
      auto e = log::xml::text::make("hello world");
      expect(make_string(*e), equal_to("hello world\n"));
    });
  });

  subsuite<log::xml::document>(_, "document", bind_factory("root"),
                               [](auto &_) {
    _.test("empty root", [](log::xml::document &doc) {
      expect(make_string(doc), equal_to(XML "<root/>\n"));
    });

    _.test("single child", [](log::xml::document &doc) {
      doc.root()->append_child(log::xml::element::make("child"));
      expect(make_string(doc), equal_to(XML "<root>\n  <child/>\n</root>\n"));
    });

    _.test("complex", [](log::xml::document &doc) {
      auto child = log::xml::element::make("child");
      child->attr("attr", "value");
      child->append_child(log::xml::text::make("hello world"));
      doc.root()->append_child(std::move(child));
      expect(make_string(doc), equal_to(
        XML
        "<root>\n"
        "  <child attr=\"value\">\n"
        "    hello world\n"
        "  </child>\n"
        "</root>\n"
      ));
    });
  });
});
