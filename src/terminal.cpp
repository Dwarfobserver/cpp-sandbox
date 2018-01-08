
#include <terminal.hpp>

using namespace std::literals;

namespace sc {

    std::vector<std::string> terminal::parse_line(std::string const &line) {
        std::vector<std::string> args;

        bool inMarks = false;
        bool inArg = false;
        char const* it = line.data();
        char const* begin;
        while (it != line.data() + line.size()) {
            if (inArg) {
                if (*it == '\'') {
                    inMarks = !inMarks;
                    continue;
                }
                if (inMarks) continue;
                if (*it == ' ') {
                    args.emplace_back(begin, it);
                    inArg = false;
                }
            }
            else {
                if (*it == ' ') continue;
                begin = it;
                inArg = true;
            }
            ++it;
        }

        return args;
    }


    void terminal::async_wait_line() {
        line_ = std::async(std::launch::async, [] {
            std::string line;
            std::getline(std::cin, line);
            return line;
        });
    }

    bool terminal::is_line_available() {
        auto status = line_.wait_for(0s);
        return status == std::future_status::ready;
    }

    std::string terminal::get_line() {
        return line_.get();
    }

}
