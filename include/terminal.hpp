
#pragma once

#include <future>
#include <string>
#include <iostream>
#include <vector>


namespace sc {

    class terminal;
    template <class T>
    terminal& operator<<(terminal& terminal, T const& data);

    class terminal {
        template <class T>
        friend terminal& operator<<(terminal& terminal, T const& data);
    public:
        static std::vector<std::string> parse_line(std::string const& line);

        void async_wait_line();
        bool is_line_available();
        std::string get_line();
    private:
        std::future<std::string> line_;
    };

    template<class T>
    terminal &operator<<(terminal& terminal, const T &data) {
        std::cout << data;
        return terminal;
    }

}
