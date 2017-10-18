
cmake_minimum_required(VERSION 3.8)
project(Skappie)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE_FILES
        include/utils.hpp
        include/spsc_queue.hpp
        include/old_fs_alloc.hpp

        tests/catch.hpp
        tests/main.cpp
        tests/test_mpsc_queue.cpp)

add_executable(Tests ${SOURCE_FILES})
