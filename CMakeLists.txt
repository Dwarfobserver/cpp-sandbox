
cmake_minimum_required(VERSION 3.8)
project(cpp-sandbox)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE_FILES

        # Includes

        include/utils.hpp
        include/fluent_collections.hpp
        include/spsc_queue.hpp
        include/mpsc_queue.hpp
        include/slot_map.hpp
        include/block_allocator.hpp
        include/lazy_ranges.hpp
        include/logger.hpp
        include/transactional.hpp
        include/optional.hpp
        include/stack_allocator.hpp
        include/pointer_iterators.hpp
        include/flag_enums.hpp

        include/udp/traits.hpp
        include/udp/packet.hpp
        include/udp/listener.hpp
        include/udp/sender.hpp

        # Tests

        tests/catch.hpp
        tests/tests_performances.cpp
        tests/tests_utils.cpp
        tests/tests_fluent_collections.cpp
        tests/tests_spsc_queue.cpp
        tests/tests_mpsc_queue.cpp
        tests/tests_slot_map.cpp
        tests/tests_block_allocator.cpp
        tests/tests_lazy_ranges.cpp
        tests/tests_transactional.cpp
        tests/tests_optional.cpp
        tests/tests_flag_enums.cpp

#        tests/udp_socket_impl.hpp
#        tests/tests_udp_sockets.cpp
        )

add_executable(Tests ${SOURCE_FILES})
