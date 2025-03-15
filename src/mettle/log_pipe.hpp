#ifndef INC_METTLE_SRC_LOG_PIPE_HPP
#define INC_METTLE_SRC_LOG_PIPE_HPP

#include <istream>

#include <bencode.hpp>

#include <mettle/driver/log/core.hpp>

namespace mettle::log {

  class pipe {
  public:
    pipe(log::file_logger &logger, test_uid file_uid)
      : logger_(logger), file_uid_(file_uid) {}

    void operator ()(std::istream &s) {
      auto tmp = bencode::decode(s, bencode::no_check_eof);
      auto &data = std::get<bencode::dict>(tmp);
      auto &&event = std::get<bencode::string>(data.at("event"));

      if(event == "started_suite") {
        logger_.started_suite(read_suites( std::move(data.at("suites")) ));
      } else if(event == "ended_suite") {
        logger_.ended_suite(read_suites( std::move(data.at("suites")) ));
      } else if(event == "started_test") {
        logger_.started_test(read_test_name( std::move(data.at("test")) ));
      } else if(event == "passed_test") {
        logger_.passed_test(
          read_test_name( std::move(data.at("test")) ),
          read_test_output( std::move(data.at("output")) ),
          read_test_duration( std::move(data.at("duration")) )
        );
      } else if(event == "failed_test") {
        logger_.failed_test(
          read_test_name( std::move(data.at("test")) ),
          read_string( std::move(data.at("message")) ),
          read_test_output( std::move(data.at("output")) ),
          read_test_duration( std::move(data.at("duration")) )
        );
      } else if(event == "skipped_test") {
        logger_.skipped_test(read_test_name( std::move(data.at("test")) ),
                             read_string( std::move(data.at("message"))) );
      } else if(event == "failed_file") {
        logger_.failed_file(
          {read_string( std::move(data.at("file")) ), file_uid_},
          read_string( std::move(data.at("message")) )
        );
      }
    }
  private:
    std::vector<std::string> read_suites(bencode::data &&suites) {
      std::vector<std::string> result;
      for(auto &&i : std::get<bencode::list>(suites))
        result.push_back(std::move( std::get<bencode::string>(i) ));
      return result;
    }

    test_name read_test_name(bencode::data &&test) {
      auto &data = std::get<bencode::dict>(test);

      // Make sure every test has a unique ID, even if some files have
      // overlapping IDs.
      test_uid id = file_uid_ + static_cast<test_uid>(
        std::get<bencode::integer>(data.at("id"))
      );
      return {
        read_suites( std::move(data.at("suites")) ),
        std::move(std::get<bencode::string>( data.at("test") )),
        id,
        std::move(std::get<bencode::string>( data.at("file") )),
        std::move(std::get<bencode::integer>( data.at("line") )),
      };
    }

    log::test_output read_test_output(bencode::data &&output) {
      auto &data = std::get<bencode::dict>(output);
      return log::test_output{
        std::move(std::get<bencode::string>( data.at("stdout_log") )),
        std::move(std::get<bencode::string>( data.at("stderr_log") ))
      };
    }

    log::test_duration read_test_duration(bencode::data &&duration) {
      return log::test_duration(std::get<bencode::integer>(duration));
    }

    std::string read_string(bencode::data &&message) {
      return std::move(std::get<bencode::string>(message));
    }

    log::file_logger &logger_;
    test_uid file_uid_;
  };

} // namespace mettle::log

#endif
