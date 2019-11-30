#ifndef INC_METTLE_DRIVER_LOG_INDENT_HPP
#define INC_METTLE_DRIVER_LOG_INDENT_HPP

#include <cassert>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

namespace mettle {

  enum class indent_style {
    visual,
    logical
  };

  template<typename Char, typename Traits = std::char_traits<Char>>
  class basic_indenting_streambuf : public std::basic_streambuf<Char, Traits> {
  public:
    using base_type = std::basic_streambuf<Char, Traits>;
    using int_type = typename base_type::int_type;

    basic_indenting_streambuf(base_type *buf, std::size_t base_indent = 2)
      : buf_(buf), base_indent_(base_indent) {}

    void indent(std::ptrdiff_t offset, indent_style style) {
      if(style == indent_style::logical)
        offset *= base_indent_;

      if(offset < 0 && indent_ < static_cast<std::size_t>(-offset))
        indent_ = 0;
      else
        indent_ += offset;
    }
  protected:
    int_type overflow(int_type ch) override {
      if(new_line_ && ch != '\n') {
        for(std::size_t i = 0; i != indent_; i++)
          buf_->sputc(' ');
      }
      new_line_ = ch == '\n';
      return buf_->sputc(ch);
    }

    int sync() override {
      return buf_->pubsync();
    }
  private:
    std::basic_streambuf<Char, Traits> *buf_;
    std::size_t base_indent_;
    std::size_t indent_ = 0;
    bool new_line_ = true;
  };

  using indenting_streambuf = basic_indenting_streambuf<char>;

  template<typename Char, typename Traits = std::char_traits<Char>>
  class basic_indenting_ostream : public std::basic_ostream<Char, Traits> {
  public:
    using base_type = std::basic_ostream<Char, Traits>;
    using streambuf_type = basic_indenting_streambuf<Char, Traits>;

    explicit basic_indenting_ostream(base_type &os, std::size_t base_indent = 2)
      : base_type(&buf), buf(os.rdbuf(), base_indent) {
      base_type::copyfmt(os);
      base_type::clear(os.rdstate());
    }

    basic_indenting_ostream(basic_indenting_ostream &&rhs)
      : base_type(std::move(rhs)), buf(std::move(rhs.buf)) {
      base_type::set_rdbuf(&buf);
    }

    basic_indenting_ostream & operator =(basic_indenting_ostream &&rhs) {
      base_type::operator =(std::move(rhs));
      buf = std::move(rhs.buf);
      return *this;
    }

    void indent(std::ptrdiff_t offset, indent_style style) {
      buf.indent(offset, style);
    }
  private:
    streambuf_type buf;
  };

  using indenting_ostream = basic_indenting_ostream<char>;

  template<typename Char, typename Traits = std::char_traits<Char>>
  class basic_scoped_indent {
  public:
    using stream_type = basic_indenting_ostream<Char, Traits>;
    basic_scoped_indent(
      stream_type &os, indent_style style = indent_style::logical,
      std::ptrdiff_t depth = 1
    ) : os_(os), style_(style), depth_(depth) {
      assert(depth_ >= 0);
      os_.indent(depth_, style_);
    }

    ~basic_scoped_indent() {
      os_.indent(-depth_, style_);
    }
  private:
    stream_type &os_;
    indent_style style_;
    std::ptrdiff_t depth_;
  };

  using scoped_indent = basic_scoped_indent<char>;

  template<typename Char, typename Traits = std::char_traits<Char>>
  class basic_indenter {
  public:
    using stream_type = basic_indenting_ostream<Char, Traits>;
    basic_indenter(
      stream_type &os, indent_style style = indent_style::logical,
      std::ptrdiff_t depth = 0
    ) : os_(os), style_(style), depth_(depth) {
      assert(depth_ >= 0);
      os_.indent(depth_, style_);
    }

    ~basic_indenter() {
      os_.indent(-depth_, style_);
    }

    void operator ++() {
      os_.indent(1, style_);
      depth_++;
    }
    void operator ++(int) {
      operator ++();
    }

    void operator --() {
      if(depth_ > 0) {
        os_.indent(-1, style_);
        depth_--;
      }
    }
    void operator --(int) {
      operator --();
    }

    void reset() {
      os_.indent(-depth_, style_);
      depth_ = 0;
    }
  private:
    stream_type &os_;
    indent_style style_;
    std::ptrdiff_t depth_;
  };

  using indenter = basic_indenter<char>;

} // namespace mettle

#endif
