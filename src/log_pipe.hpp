#ifndef INC_METTLE_SRC_LOG_PIPE_HPP
#define INC_METTLE_SRC_LOG_PIPE_HPP

#include <istream>

#include <bencode.hpp>

#include <mettle/driver/log/core.hpp>

namespace mettle {

namespace log {

  class pipe {
  public:
    explicit pipe(log::file_logger &logger) : logger_(logger) {}

    void operator ()(std::istream &s) {
      using namespace BENCODE_ANY_NS;

      auto tmp = bencode::decode(s, bencode::no_check_eof);
      auto &data = any_cast<bencode::dict &>(tmp);
      auto &event = any_cast<bencode::string &>(data.at("event"));

      if(event == "started_suite") {
        logger_.started_suite(read_suites(data.at("suites")));
      }
      else if(event == "ended_suite") {
        logger_.ended_suite(read_suites(data.at("suites")));
      }
      else if(event == "started_test") {
        logger_.started_test(read_test_name(data.at("test")));
      }
      else if(event == "passed_test") {
        logger_.passed_test(read_test_name(data.at("test")),
                            read_test_output(data.at("output")),
                            read_test_duration(data.at("duration")));
      }
      else if(event == "failed_test") {
        logger_.failed_test(read_test_name(data.at("test")),
                            read_string(data.at("message")),
                            read_test_output(data.at("output")),
                            read_test_duration(data.at("duration")));
      }
      else if(event == "skipped_test") {
        logger_.skipped_test(read_test_name(data.at("test")),
                             read_string(data.at("message")));
      }
      else if(event == "failed_file") {
        logger_.failed_file(read_string(data.at("file")),
                            read_string(data.at("message")));
      }
    }

    void started_file(const std::string &file) {
      logger_.started_file(file);
    }

    void ended_file(const std::string &file) {
      logger_.ended_file(file);
      file_index_++;
    }

    void failed_file(const std::string &file, const std::string &message) {
      logger_.failed_file(file, message);
      file_index_++;
    }
  private:
    std::vector<std::string> read_suites(BENCODE_ANY_NS::any &suites) {
      using namespace BENCODE_ANY_NS;
      std::vector<std::string> result;
      for(auto &&i : any_cast<bencode::list &>(suites))
        result.push_back(std::move( any_cast<bencode::string &>(i) ));
      return result;
    }

    test_name read_test_name(BENCODE_ANY_NS::any &test) {
      using namespace BENCODE_ANY_NS;
      auto &data = any_cast<bencode::dict &>(test);

      // Make sure every test has a unique ID, even if some files have
      // overlapping IDs.
      uint64_t id = (file_index_ << 32) + static_cast<uint64_t>(
        any_cast<bencode::integer>(data.at("id"))
      );
      return {
        read_suites(data.at("suites")),
        std::move(any_cast<bencode::string &>( data.at("test") )),
        id
      };
    }

    log::test_output read_test_output(BENCODE_ANY_NS::any &output) {
      using namespace BENCODE_ANY_NS;
      auto &data = any_cast<bencode::dict &>(output);
      return log::test_output{
        std::move(any_cast<bencode::string &>( data.at("stdout") )),
        std::move(any_cast<bencode::string &>( data.at("stderr") ))
      };
    }

    log::test_duration read_test_duration(BENCODE_ANY_NS::any &duration) {
      using namespace BENCODE_ANY_NS;
      return log::test_duration(any_cast<bencode::integer>(duration));
    }

    std::string read_string(BENCODE_ANY_NS::any &message) {
      using namespace BENCODE_ANY_NS;
      return std::move(any_cast<bencode::string &>(message));
    }

    log::file_logger &logger_;
    uint64_t file_index_ = 0;
  };

}

} // namespace mettle

#endif
