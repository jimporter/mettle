#ifndef INC_METTLE_DRIVER_LOG_INDENT_HPP
#define INC_METTLE_DRIVER_LOG_INDENT_HPP

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

  basic_indenting_streambuf(base_type *buf, size_t base_indent = 2)
    : buf_(buf), base_indent_(base_indent) {}

  void indent(ssize_t offset, indent_style style) {
    if(style == indent_style::logical)
      offset *= base_indent_;

    if(offset < 0 && indent_ < static_cast<size_t>(-offset))
      indent_ = 0;
    else
      indent_ += offset;
  }
protected:
  virtual int_type overflow(int_type ch) {
    if(new_line_ && ch != '\n') {
      for(size_t i = 0; i != indent_; i++)
        buf_->sputc(' ');
    }
    new_line_ = ch == '\n';
    return buf_->sputc(ch);
  }

  virtual int sync() {
    return buf_->pubsync();
  }
private:
  std::basic_streambuf<Char, Traits> *buf_;
  size_t base_indent_;
  size_t indent_ = 0;
  bool new_line_ = true;
};

using indenting_streambuf = basic_indenting_streambuf<char>;

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_indenting_ostream : public std::basic_ostream<Char, Traits> {
public:
  using base_type = std::basic_ostream<Char, Traits>;
  using streambuf_type = basic_indenting_streambuf<Char, Traits>;

  basic_indenting_ostream(base_type &os, size_t base_indent = 2)
    : base_type(&buf), buf(os.rdbuf(), base_indent) {
    this->copyfmt(os);
    this->clear(os.rdstate());
  }

  void indent(ssize_t offset, indent_style style) {
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
    size_t depth = 1
  ) : os_(os), style_(style), depth_(depth) {
    os_.indent(depth_, style_);
  }

  ~basic_scoped_indent() {
    os_.indent(-depth_, style_);
  }
private:
  stream_type &os_;
  indent_style style_;
  size_t depth_;
};

using scoped_indent = basic_scoped_indent<char>;

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_indenter {
public:
  using stream_type = basic_indenting_ostream<Char, Traits>;
  basic_indenter(
    stream_type &os, indent_style style = indent_style::logical,
    size_t depth = 0
  ) : os_(os), style_(style), depth_(depth) {
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
  size_t depth_;
};

using indenter = basic_indenter<char>;

} // namespace mettle

#endif
