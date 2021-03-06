
cmake_minimum_required(VERSION 3.8)
project(cpp-sandbox)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE_FILES

        # Includes

        include/fluent_collections.hpp
        include/spsc_queue.hpp
        include/mpsc_queue.hpp
        include/slot_map.hpp
        include/block_allocator.hpp
        include/lazy_ranges.hpp
        include/transactional.hpp
        include/fast_optional.hpp
        include/pointer_iterators.hpp
        include/flag_enums.hpp
        include/serializer_span.hpp
        include/pod_vector.hpp
        include/terminal.hpp src/terminal.cpp
        include/make_string.hpp
        include/monad.hpp
        include/type_traits.hpp
        include/eval.hpp src/eval.cpp
        include/movable_function.hpp
        include/compact_map.hpp
        include/thread_pool.hpp src/thread_pool.cpp
        include/stack_array.hpp
        include/stack_tracker.hpp src/stack_tracker.cpp
        include/bytes_units.hpp
        include/stack_allocator.hpp
        include/arc_garbage_collector.hpp
        include/compiler_hints.hpp

        # Tests

        tests/catch.hpp
        tests/tests_performances.cpp
        tests/tests_fluent_collections.cpp
        tests/tests_spsc_queue.cpp
        tests/tests_mpsc_queue.cpp
        tests/tests_slot_map.cpp
        tests/tests_block_allocator.cpp
        tests/tests_lazy_ranges.cpp
        tests/tests_transactional.cpp
        tests/tests_fast_optional.cpp
        tests/tests_flag_enums.cpp
        tests/tests_serializer_span.cpp
        tests/tests_pod_vector.cpp
        tests/tests_make_string.cpp
        tests/tests_stack_allocator.cpp
        tests/tests_monad.cpp
        tests/tests_type_traits.cpp
        tests/tests_eval.cpp
        tests/tests_movable_function.cpp
        tests/tests_compact_map.cpp
        tests/tests_thread_pool.cpp
        tests/tests_stack_array.cpp
        tests/tests_stack_tracker.cpp
        tests/tests_arc_garbage_collector.cpp
        tests/tests_compiler_hints.cpp
        )

add_executable(Tests ${SOURCE_FILES})
