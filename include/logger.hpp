
#pragma once

#include <mutex>
#include <iostream>
#include <atomic>


namespace sc {

    class logger {
        using lock_t = std::lock_guard<std::mutex>;
        friend class output;
    public:
        output debug;
        output info;
        output warning;
        output error;

        explicit logger(std::ostream& os);

        void flush();
        void set_level(level::type level);

        logger(logger&& moved) = delete;

        struct level {
            enum type : int {
                debug = 0,
                info,
                warning,
                error,
                disabled
            };
        };

        class output {
            friend class logger;
        public:
            template<class Msg>
            void operator<<(Msg &&msg) const;

            void set_header(std::string const& header);

            output(output const &clone) = delete;
            output(output &&clone) = delete;
            output &operator=(output const &clone) = delete;
            output &operator=(output &&clone) = delete;
        private:
            logger &_logger;
            level::type _level;
            std::string _header;

            output(logger &logger, level::type level, std::string &&header) :
                    _logger(logger),
                    _level(level),
                    _header(std::move(header)) {}
        };

    private:
        std::atomic<level::type> _level;
        std::ostream* _pOstream;
        std::mutex _ostreamMutex;
    };

    logger::logger(std::ostream& os) :
            debug(  *this, level::debug,   "[Debug]   "),
            info(   *this, level::info,    "[Info]    "),
            warning(*this, level::warning, "[Warning] "),
            error(  *this, level::error,   "[Error]   "),
            _level(level::debug),
            _pOstream(&os)
    {}

    void logger::set_level(logger::level::type level) {
        _level.store(level);
    }

    void logger::flush() {
        lock_t lock(_ostreamMutex);
        *_pOstream << std::flush;
    }

    template<class Msg>
    void logger::output::operator<<(Msg &&msg) const {
        if (_level < _logger._level.load()) return;
        lock_t lock(_logger._ostreamMutex);
        *_logger._pOstream << _header << std::forward<Msg>(msg) << '\n';
    }

    void logger::output::set_header(std::string const &header) {
        lock_t lock(_logger._ostreamMutex);
        _header = header;
    }

}
