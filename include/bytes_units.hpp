
#pragma once

#include <cstdint>

namespace sc {

    namespace bytes_units {
        enum : uint64_t {
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
        inline uint64_t operator "" _KB(uint64_t val) { return val * bytes_units::kilobytes; }
        inline uint64_t operator "" _MB(uint64_t val) { return val * bytes_units::megabytes; }
        inline uint64_t operator "" _GB(uint64_t val) { return val * bytes_units::gigabytes; }
        inline uint64_t operator "" _TB(uint64_t val) { return val * bytes_units::terabytes; }
        inline uint64_t operator "" _PB(uint64_t val) { return val * bytes_units::petabytes; }
        inline uint64_t operator "" _EB(uint64_t val) { return val * bytes_units::exabytes; }
        inline uint64_t operator "" _ZB(uint64_t val) { return val * bytes_units::zettabytes; }
    }

}
