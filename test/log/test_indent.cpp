#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/indent.hpp>

struct streambuf_factory {
  streambuf_factory() : sbuf(std::ios_base::out), ibuf(&sbuf) {}
  std::stringbuf sbuf;
  indenting_streambuf ibuf;
};

struct stream_factory {
  stream_factory() : is(ss) {}
  std::ostringstream ss;
  indenting_ostream is;
};

suite<> test_indent("indentation", [](auto &_) {

  subsuite<streambuf_factory>(_, "indenting_streambuf", [](auto &_) {
    auto generate = [](streambuf_factory &f, indent_style a, indent_style b) {
      f.ibuf.indent(1, a);
      f.ibuf.sputn("1", 1);
      f.ibuf.sputn("2", 1);
      f.ibuf.indent(1, b);
      f.ibuf.sputn("3\n", 2);
      f.ibuf.sputn("4\n", 2);
      f.ibuf.indent(-1, b);
      f.ibuf.sputn("5\n6", 3);
    };

    _.test("logical indentation", [generate](streambuf_factory &f) {
      generate(f, indent_style::logical, indent_style::logical);
      expect(f.sbuf.str(), equal_to("  123\n    4\n  5\n  6"));
    });

    _.test("visual indentation", [generate](streambuf_factory &f) {
      generate(f, indent_style::visual, indent_style::visual);
      expect(f.sbuf.str(), equal_to(" 123\n  4\n 5\n 6"));
    });

    _.test("mixed indentation", [generate](streambuf_factory &f) {
      generate(f, indent_style::logical, indent_style::visual);
      expect(f.sbuf.str(), equal_to("  123\n   4\n  5\n  6"));
    });
  });

  subsuite<stream_factory>(_, "indenting_ostream", [](auto &_) {
    auto generate = [](stream_factory &f, indent_style a, indent_style b) {
      f.is.indent(1, a);
      f.is << "1" << "2";
      f.is.indent(1, b);
      f.is << "3\n" << "4\n";
      f.is.indent(-1, b);
      f.is << "5\n6";
    };

    _.test("logical indentation", [generate](stream_factory &f) {
      generate(f, indent_style::logical, indent_style::logical);
      expect(f.ss.str(), equal_to("  123\n    4\n  5\n  6"));
    });

    _.test("visual indentation", [generate](stream_factory &f) {
      generate(f, indent_style::visual, indent_style::visual);
      expect(f.ss.str(), equal_to(" 123\n  4\n 5\n 6"));
    });

    _.test("mixed indentation", [generate](stream_factory &f) {
      generate(f, indent_style::logical, indent_style::visual);
      expect(f.ss.str(), equal_to("  123\n   4\n  5\n  6"));
    });
  });

  subsuite<stream_factory>(_, "scoped_indent", [](auto &_) {
    auto generate = [](stream_factory &f, indent_style a, indent_style b) {
      f.is << "1";
      {
        scoped_indent i(f.is, a);
        f.is << "2\n3";
        {
          scoped_indent ii(f.is, b);
          f.is << "4\n5";
        }
      }
      f.is << "6\n7";
    };

    _.test("logical indentation", [generate](stream_factory &f) {
      generate(f, indent_style::logical, indent_style::logical);
      expect(f.ss.str(), equal_to("12\n  34\n    56\n7"));
    });

    _.test("visual indentation", [generate](stream_factory &f) {
      generate(f, indent_style::visual, indent_style::visual);
      expect(f.ss.str(), equal_to("12\n 34\n  56\n7"));
    });

    _.test("mixed indentation", [generate](stream_factory &f) {
      generate(f, indent_style::logical, indent_style::visual);
      expect(f.ss.str(), equal_to("12\n  34\n   56\n7"));
    });
  });

  subsuite<stream_factory>(_, "indenter", [](auto &_) {
    auto generate = [](stream_factory &f, indent_style style) {
      indenter i(f.is, style);
      f.is << "1";
      i++;
      f.is << "2\n3";
      ++i; ++i;
      f.is << "4\n5";
      i--;
      f.is << "6\n7";
      --i;
      f.is << "8\n9";
      i.reset();
      f.is << "a\nb";
    };

    _.test("logical indentation", [generate](stream_factory &f) {
      generate(f, indent_style::logical);
      expect(f.ss.str(), equal_to("12\n  34\n      56\n    78\n  9a\nb"));
    });

    _.test("visual indentation", [generate](stream_factory &f) {
      generate(f, indent_style::visual);
      expect(f.ss.str(), equal_to("12\n 34\n   56\n  78\n 9a\nb"));
    });
  });
});
