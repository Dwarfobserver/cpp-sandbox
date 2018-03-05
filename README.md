
# cpp-sandbox

This is a repository containing some C++ code I developed.
It uses some C++17 features (some others are missing from mingw).
Tests are run with Catch and can be used as a poor documentation.
Here are the copyable includes, sorted by usefulness or interest :

### Reusable includes :

 - movable_function : A wrapper which can accept any movable callable, with allocation optimisation for function pointers. It is useful for remplacing std::function when the functions dosen't need to be copied.

 - make_string : A generic function which will attempt to conveft any type to a string. In particular, it works for tuples or pairs, iterables, and types with a to_string function (std::to_string i also tested).

 - arc_garbage_collector :  A single thread garbage collector (the collect can be asynchronous) based on reference counting. It can use any allocator given, rebound to an internal node class. It performs only one allocation per object created. Do not works for over-aligned types.
 
 - compiler_hints : Macros for code optimisation and self-documentation make cross-platform for gcc, clang and msvc. Defines ASSERT(x, msg), LIKELY(x), UNLIKELY(x), UNREACHABLE(), RESTRICT, FORCE_INLINE and NO_INLINE for gcc, clang and msvc (tested on godbolt.org).
      
 - slot_map : A structure which can add and remove elements from their id in O(1), and store them in contiguous memory. It is build upon std::vector.
 
 - spsc_queue : Wait-free single producer & single consumer queue. This class do not check for overflow (it have a good chance to throw in debug mode).

 - transactional : A lock-free linked list storing successives versions of a value copied when modified. It allows to get the value without wait. Values destructions are deferred to a 'clear' function.

 - pod_vector : A fast version of std::vector which doesn't construct or destroy it's elements (useful for bytes array for exemple).

 - monad : Let compose functions for monad types, with the operator '|' in the namespace sc::monad_operator. These types can be added by specializing the 'monad_traits' template class. std::optional and containers (iterables and with emplace, emplace_back or emplace_front) have a monad_trait specialized.

 - stack_array : Array of dynamic size created on the stack, with a similar interface to std::array.

 - stack_allocator : A fast allocator with an allocated area which can only grow.

 - pointer_iterators : Template class helpers to create pointer iterators for collections with continuous storage.
          
 - fast_optional : A template class used to define fast, compile-time optional with a type and the type value considered 'empty'.

 - fluent_collections : Functional-style wrapper of "collections", which are template classes with functions begin(), end() and insert(iterator) that behave gently. Not performant, see 'lazy_ranges' include.
 
 - bytes_units : Contains an enumeration for ratios and literal operators for units between kilobytes and zettabytes.

 - flag_enums : Namespace with bit operators for enum classes.

### Incomplete includes :

 - serializer_span : Binary serialization functions using non-owned memory with a simple implementation for the client, and, if the type allows it, deducing the serialized size at compile-time. The core is functional, but it need some basic types, optimizations and traits to be mature.

 - type_traits : Few traits, for detecting iterators, iterables, and 'emplace-able' classes (with emplace_front, emplace_back or emplace). Need to recognize built_in arrays as iterables.

 - mpsc_queue : Lock-free multiple producer & single (wait-free) consumer queue. This class is safe from overflow, but can then block producers. Probable culprit of rare slowdowns in tests.

 - compact_map : A map built upon std::vector for cache efficiency (for iterations and search). std::bad_alloc in release mode.

 - lazy_ranges : A version of fluent_collections with lazy evaluation. Need better performances (mostly by removing intermediate optionals).

 - block_allocator : A fast allocator for one object at a time of a fixed class. Need to accept other classes with acceptable alignment and size constraints.

 - thread_pool : A thread pool which can executes given tasks. Working, but miss some functionnalities. Triggered one segmentation fault.

 - terminal : Object representing the terminal, used to write, display information, wait and interpret user commands. It is almost empty for now.

 - eval : A function which compile and launch a process with the source code given. This is not cross-platform nor efficient, it does not have interoperability with another (or the current) process, and it's steps cannot be separated.

 - stack_tracker : Static functions : one initialize the tracker with the stack capacity and the other determine if a memory address belongs to the stack. Not precise, may fail.
