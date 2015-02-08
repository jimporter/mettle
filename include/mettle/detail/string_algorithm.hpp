#ifndef INC_METTLE_DETAIL_STRING_ALGORITHM_HPP
#define INC_METTLE_DETAIL_STRING_ALGORITHM_HPP

#include <initializer_list>
#include <iterator>
#include <ostream>
#include <string>

namespace mettle {

namespace detail {

  struct identity {
    template<typename T>
    constexpr decltype(auto) operator ()(T &&t) const {
      return std::forward<T>(t);
    }
  };

  template<typename T>
  std::string stringify(const T &t) {
    return (std::ostringstream{} << t).str();
  }

  template<typename ...Args>
  const std::string & stringify(const std::string &s) {
    return s;
  }

  class ostream_list_append {
  public:
    ostream_list_append(std::ostream &os, std::string delim = ", ")
      : os_(os), delim_(std::move(delim)) {}

    template<typename T>
    void operator ()(T &&t) {
      if(first_)
        first_ = false;
      else
        os_ << delim_;
      os_ << std::forward<T>(t);
    }
  private:
    std::ostream &os_;
    const std::string delim_;
    bool first_ = true;
  };

  template<typename Iter, typename Func>
  class string_joiner {
  public:
    string_joiner(Iter begin, Iter end, const Func &func,
                  const std::string &delim)
      : begin_(begin), end_(end), func_(func), delim_(delim) {}
    string_joiner(const string_joiner &) = delete;

    friend std::ostream &
    operator <<(std::ostream &os, const string_joiner &t) {
      ostream_list_append append(os, t.delim_);
      for(auto i = t.begin_; i != t.end_; ++i)
        append(t.func_(*i));
      return os;
    }
  private:
    const Iter begin_, end_;
    const Func &func_;
    const std::string &delim_;
  };

  template<typename Iter, typename Func = identity>
  inline const string_joiner<Iter, Func>
  iter_joined(Iter begin, Iter end, const Func &func = identity{},
         const std::string &delim = ", ") {
    return {begin, end, func, delim};
  }

  template<typename T, typename Func = identity>
  inline auto joined(const T &t, const Func &func = identity{},
                     const std::string &delim = ", ") ->
  const string_joiner<decltype(std::begin(t)), Func> {
    return {std::begin(t), std::end(t), func, delim};
  }

  template<typename T, typename Func = identity>
  inline auto joined(std::initializer_list<T> t, const Func &func = identity{},
                     const std::string &delim = ", ") ->
  const string_joiner<decltype(std::begin(t)), Func> {
    return {std::begin(t), std::end(t), func, delim};
  }

}

} // namespace mettle

#endif
