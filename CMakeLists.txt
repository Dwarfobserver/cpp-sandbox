
cmake_minimum_required(VERSION 3.8)
project(Skappie)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE_FILES
        include/utils.hpp
        include/fluent_collections.hpp
        include/spsc_queue.hpp
        include/mpsc_queue.hpp
        include/transactional_map.hpp

        tests/catch.hpp
        tests/main.cpp
        tests/tests_utils.cpp
        tests/tests_fluent_collections.cpp
        tests/tests_spsc_queue.cpp
        tests/tests_spmc_queue.cpp
        tests/tests_transactional_map.cpp)

add_executable(Tests ${SOURCE_FILES})
