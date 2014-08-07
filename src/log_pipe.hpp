#ifndef INC_METTLE_PIPE_LOG_HPP
#define INC_METTLE_PIPE_LOG_HPP

#include <istream>

#include <bencode.hpp>

#include <mettle/log/core.hpp>

namespace mettle {

namespace log {

  class pipe {
  public:
    explicit pipe(log::test_logger &logger) : logger_(logger) {}

    void operator ()(std::istream &s) {
      using namespace BENCODE_ANY_NS;

      auto tmp = bencode::decode(s);
      if(tmp.empty())
        return;

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
                            read_test_output(data.at("output")));
      }
      else if(event == "failed_test") {
        logger_.failed_test(read_test_name(data.at("test")),
                            read_test_message(data.at("message")),
                            read_test_output(data.at("output")));
      }
      else if(event == "skipped_test") {
        logger_.skipped_test(read_test_name(data.at("test")));
      }
    }
  private:
    std::vector<std::string> read_suites(BENCODE_ANY_NS::any &suites) {
      using namespace BENCODE_ANY_NS;
      std::vector<std::string> result;
      for(auto &&i : any_cast<bencode::list &>(suites))
        result.push_back(std::move( any_cast<bencode::string &>(i) ));
      return result;
    }

    log::test_name read_test_name(BENCODE_ANY_NS::any &test) {
      using namespace BENCODE_ANY_NS;
      auto &data = any_cast<bencode::dict &>(test);
      return log::test_name{
        read_suites(data.at("suites")),
        std::move(any_cast<bencode::string &>( data.at("test")  )),
        static_cast<size_t>(any_cast<bencode::integer>( data.at("id") )),
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

    std::string read_test_message(BENCODE_ANY_NS::any &message) {
      using namespace BENCODE_ANY_NS;
      return std::move(any_cast<bencode::string &>(message));
    }

    log::test_logger &logger_;
  };

}

} // namespace mettle

#endif
