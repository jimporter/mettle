#ifndef INC_METTLE_MATCHERS_EXCEPTION_HPP
#define INC_METTLE_MATCHERS_EXCEPTION_HPP

#include "core.hpp"

namespace mettle {

  namespace detail {

    template<typename Exception, typename Matcher>
    class thrown_impl_base : public matcher_tag {
    public:
      thrown_impl_base(Matcher matcher) : matcher(std::move(matcher)) {}

      std::string desc() const {
        std::ostringstream ss;
        ss << "threw " << type_name<Exception>() << "(" << matcher.desc()
           << ")";
        return ss.str();
      }
    protected:
      match_result match(const Exception &e) const {
        match_result m = matcher(e);
        std::ostringstream ss;
        ss << "threw ";
        if(m.message.empty())
          ss << to_printable(e);
        else
          ss << type_name(e) << "(" << m.message << ")";
        return {m.matched, ss.str()};
      }
    private:
      Matcher matcher;
    };


    template<typename Exception, typename Matcher>
    struct thrown_impl : thrown_impl_base<Exception, Matcher> {
      using base = thrown_impl_base<Exception, Matcher>;

      using base::base;

      template<typename U>
      match_result operator ()(U &&actual) const {
        try {
          actual();
          return {false, "threw nothing"};
        } catch(const Exception &e) {
          return base::match(e);
        } catch(const std::exception &e) {
          std::ostringstream ss;
          ss << "threw " << to_printable(e);
          return {false, ss.str()};
        } catch(...) {
          return {false, "threw unknown exception"};
        }
      }
    };

    template<typename Matcher>
    struct thrown_impl<std::exception, Matcher>
      : thrown_impl_base<std::exception, Matcher> {
      using base = thrown_impl_base<std::exception, Matcher>;

      using base::base;

      template<typename U>
      match_result operator ()(U &&actual) const {
        try {
          actual();
          return {false, "threw nothing"};
        } catch(const std::exception &e) {
          return base::match(e);
        } catch(...) {
          return {false, "threw unknown exception"};
        }
      }
    };

  } // namespace detail

  template<typename Exception, typename T>
  auto thrown_raw(T &&thing) {
    return detail::thrown_impl<Exception, ensure_matcher_t<T>>(
      ensure_matcher(std::forward<T>(thing))
    );
  }

  template<typename T>
  auto exception_what(T &&thing) {
    return filter(
      [](auto &&e) { return std::string(e.what()); },
      ensure_matcher(std::forward<T>(thing)),
      "what: "
    );
  }

  template<typename Exception, typename T>
  auto thrown(T &&thing) {
    return thrown_raw<Exception>(exception_what(std::forward<T>(thing)));
  }

  template<typename Exception>
  auto thrown() {
    return thrown_raw<Exception>(anything());
  }

  inline auto thrown() {
    return basic_matcher([](auto &&actual) -> match_result {
      try {
        actual();
        return {false, "threw nothing"};
      } catch(const std::exception &e) {
        std::ostringstream ss;
        ss << "threw " << to_printable(e);
        return {true, ss.str()};
      } catch(...) {
        return {true, "threw unknown exception"};
      }
    }, "threw exception");
  }

} // namespace mettle

#endif
