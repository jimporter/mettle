#ifndef INC_METTLE_SRC_GLOB_HPP
#define INC_METTLE_SRC_GLOB_HPP

#ifndef _WIN32
#  include <glob.h>
#else
#  include <boost/iterator/iterator_facade.hpp>
#  include <windows.h>
#  include "../err_string.hpp"
#endif

namespace mettle {

#ifndef _WIN32

class glob {
public:
  using iterator = const char **;

  glob(const std::string &s) {
    int err = ::glob(s.c_str(), 0, nullptr, &glob_);
    if(err == GLOB_NOMATCH)
      throw std::runtime_error("no matches found: " + s);
    else if(err != 0)
      throw std::runtime_error("unknown error");
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

      if((glob_->handle = FindFirstFileA(s.c_str(), &glob_->data)) ==
         INVALID_HANDLE_VALUE) {
        auto err = GetLastError();
        if(err == ERROR_FILE_NOT_FOUND)
          throw std::runtime_error("no matches found: " + s);
        else
          throw std::runtime_error(err_string(err));
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
          throw std::runtime_error(err_string(err));

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

} // namespace mettle

#endif
