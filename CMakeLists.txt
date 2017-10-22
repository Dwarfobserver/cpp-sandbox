
cmake_minimum_required(VERSION 3.8)
project(Skappie)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE_FILES
        include/utils.hpp
        include/spsc_queue.hpp
        include/aligned_array.hpp
        include/transactional_map.hpp

        deprecated/fwd_list.hpp

        tests/catch.hpp
        tests/main.cpp
        tests/tests_spsc_queue.cpp tests/tests_transactional_map.cpp include/fluent_collections.hpp tests/tests_fluent_collections.cpp tests/tests_utils.cpp include/mpsc_queue.hpp tests/tests_spmc_queue.cpp)

add_executable(Tests ${SOURCE_FILES})