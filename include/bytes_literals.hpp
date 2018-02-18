
#pragma once

#include <cstdint>

namespace sc {

    namespace bytes_ratio {
        enum units {
            kilobytes = 1'024,
            megabytes = kilobytes * 1'024,
            gigabytes = megabytes * 1'024,
            terabytes = gigabytes * 1'024,
            petabytes = terabytes * 1'024,
            exabytes  = petabytes * 1'024,
            zettabytes = exabytes * 1'024,
        };
    }

    namespace bytes_literals {
        inline uint64_t operator "" _kb(uint64_t val) { return val * bytes_ratio::kilobytes; }
        inline uint64_t operator "" _mb(uint64_t val) { return val * bytes_ratio::megabytes; }
        inline uint64_t operator "" _gb(uint64_t val) { return val * bytes_ratio::gigabytes; }
        inline uint64_t operator "" _tb(uint64_t val) { return val * bytes_ratio::terabytes; }
        inline uint64_t operator "" _pb(uint64_t val) { return val * bytes_ratio::petabytes; }
        inline uint64_t operator "" _eb(uint64_t val) { return val * bytes_ratio::exabytes; }
        inline uint64_t operator "" _zb(uint64_t val) { return val * bytes_ratio::zettabytes; }
    }

}
