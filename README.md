
# cpp-sandbox

This is a repository containing some "reusable" C++ code I developed.
It uses some C++17 features (some others are missing from mingw). 

Tests are run with Catch and can be used as a poor documentation.

 - utils : Disparate things :
      - A function wrapper which defer its execution at the end of the current scope.
      - A spin lock for fast locking operations.
      - An aligned allocator, used to fill cache lines where we want to store our false sharing sensible data.

 - type_traits : Few traits, for detecting iterators, iterables, and 'emplace-able' classes (with emplace_front,
                 emplace_back or emplace).
      
 - block_allocator : A fast allocator for one object at a time of a fixed class.

 - stack_allocator : A fast allocator with an allocated area which can only grow.

 - spsc_queue : Wait-free single producer & single consumer queue.
                This class do not check for overflow (it have a good chance to throw in debug mode).

 - mpsc_queue : Lock-free multiple producer & single (wait-free) consumer queue.
                This class is safe from overflow, but can then block producers.

 - pointer_iterators : Template class helpers to create pointer iterators for collections with continuous storage.
                
 - fluent_collections : Functional-style wrapper of "collections", which are template classes with functions begin(),
                        end() and insert(iterator) that behave gently.

 - lazy_ranges : A version of fluent_collections with lazy evaluation.

 - slot_map : A structure which can add and remove elements from their id in O(1), and store them in contiguous memory.
              It is build upon std::vector.
 
 - transactional : A lock-free linked list storing successives versions of a value copied when modified. It allows to
                   get the value without wait. Values destructions are deferred to a 'clear' function.

 - optional : A template class used to define fast, compile-time optional with a type and the type value considered 'empty'.

 - serializer_span : A compile-time parametrizable span on non-owning memory for serialisation.

 - make_string : A generic function which will attempt to conveft any type to a string. In particular, it works for tuples
                 or pairs, iterables, and types with a to_string function (std::to_string is tested).

 - pod_vector : A fast version of std::vecto which doesn't construct or destroy it's elements (useful for bytes array for
                exemple).

 - monad : Let compose functions for monad types, with the operator '|' in the namespace sc::monad_operator. These types
           can be added by specializing the 'monad_traits' template class. std::optional and containers (iterables and with
           emplace, emplace_back or emplace_front) have a monad_trait specialized.

 - eval : A function which compile and lunch a process with the source code given. This is not cross-platform nor
          efficient, and it does not have interoperability with another (or the current) process.
