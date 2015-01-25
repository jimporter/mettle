#include "test_file.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#ifndef _WIN32
#  include <glob.h>
#else
#  include <boost/iterator/iterator_facade.hpp>
#  include <windows.h>
#  include "libmettle/windows/err_string.hpp"
#endif

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

namespace mettle {

namespace detail {
#ifndef _WIN32

  class glob {
  public:
    using iterator = const char ** const;

    glob(const std::string &s) {
      if(::glob(s.c_str(), 0, nullptr, &glob_) != 0)
        throw std::runtime_error("invalid glob \"" + s + "\"");
    }

    glob(const glob &) = delete;

    ~glob() {
      globfree(&glob_);
    }

    iterator begin() const {
      return const_cast<iterator>(glob_.gl_pathv);
    }

    iterator end() const {
      return const_cast<iterator>(glob_.gl_pathv + glob_.gl_pathc);
    }
  private:
    glob_t glob_;
  };

#else

  class glob {
  public:
    class iterator : public boost::iterator_facade<
      iterator, const char * const, boost::forward_traversal_tag
    > {
    public:
      iterator() = default;
      explicit iterator(const std::string &s)
        : glob_(new glob_info), filename_(glob_->data.cFileName) {

        if(!(glob_->handle = FindFirstFileA(s.c_str(), &glob_->data))) {
          auto err = GetLastError();
          if(err != ERROR_FILE_NOT_FOUND)
            throw std::runtime_error(windows::err_string(err));
        }
      }
    private:
      friend class boost::iterator_core_access;
      struct glob_info {
        ~glob_info() {
          if(handle)
            FindClose(handle);
        }

        HANDLE handle;
        WIN32_FIND_DATAA data;
      };

      void increment() {
        if(!FindNextFileA(glob_->handle, &glob_->data)) {
          auto err = GetLastError();
          if(err != ERROR_NO_MORE_FILES)
            throw std::runtime_error(windows::err_string(err));

          glob_.reset();
          filename_ = nullptr;
        }
      }

      bool equal(const iterator &rhs) const {
        if(!glob_ && !rhs.glob_)
          return true;
        return glob_->handle == rhs.glob_->handle &&
               strcmp(glob_->data.cFileName, rhs.glob_->data.cFileName) == 0;
      }

      const char * const & dereference() const {
        return filename_;
      }

      std::shared_ptr<glob_info> glob_;
      const char *filename_;
    };

    glob(const std::string &s) : value_(s) {}
    glob(const glob &) = delete;

    iterator begin() const {
      return iterator(value_);
    }

    iterator end() const {
      return iterator();
    }
  private:
    std::string value_;
  };

#endif
}

test_file::test_file(std::string command) : command_(std::move(command)) {
#ifndef _WIN32
  auto args = boost::program_options::split_unix(command_);
#else
  auto args = boost::program_options::split_winmain(command_);
#endif

  for(auto &&arg : args) {
    if(arg.empty())
      continue;

    if(arg.find_first_of("?*[") != std::string::npos) {
      detail::glob g(arg);
      std::copy(g.begin(), g.end(), std::back_inserter(args_));
    }
    else {
      args_.push_back(arg);
    }
  }
}

void validate(boost::any &v, const std::vector<std::string> &values,
              test_file*, int) {
  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string &val = validators::get_single_string(values);

  try {
    v = test_file(val);
  }
  catch(...) {
    boost::throw_exception(invalid_option_value(val));
  }
}

} // namespace mettle
