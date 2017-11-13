
cmake_minimum_required(VERSION 3.8)
project(Skappie)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)



set(SOURCE_FILES
        include/utils.hpp
        include/fluent_collections.hpp
        include/spsc_queue.hpp
        include/mpsc_queue.hpp
        include/slot_map.hpp
        include/block_allocator.hpp
        include/lazy_ranges.hpp
        include/logger.hpp

        tests/catch.hpp
        tests/tests_performances.cpp
        tests/tests_utils.cpp
        tests/tests_fluent_collections.cpp
        tests/tests_spsc_queue.cpp
        tests/tests_mpsc_queue.cpp
        tests/tests_slot_map.cpp
        tests/tests_block_allocator.cpp
        tests/tests_lazy_ranges.cpp include/transactional.hpp tests/tests_transactional.cpp include/optional.hpp tests/tests_optional.cpp)

add_executable(Tests ${SOURCE_FILES})
