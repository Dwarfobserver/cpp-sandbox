
# cpp-sandbox

This is a repository containing some ~~reusable~~ C++ code I developed.
It uses some C++17 features (some others are missing from mingw). 

Tests are run with Catch and can be used as a poor documentation.

 - utils : Few useful or funny things :
      - An operator used to safely chain calls of functions which can return an optional.
      - An aligned allocator, used to fill cache lines where we want to store our false sharing sensible data.
      - A pointer iterator (with specified padding), usable by any collection with random access.
      
 - block_allocator : An fast allocator for one object at a time, with static fixed allocation.

 - spsc_queue : Wait-free single producer & single consumer queue.
                This class do not check for overflow (it have a good chance to throw in debug mode).

 - mpsc_queue : Lock-free multiple producer & single (wait-free) consumer.
                This class is safe from overflow, but can then block producers.

 - fluent_collections : Functional-style wrapper of "collections", which are template classes with functions begin(),
                        end() and insert(iterator) that behave gently.

 - lazy_ranges : A version of fluent_collections with lazy evaluation.

 - slot_map : A structure which can add and remove elements from their id in O(1), and store them in contiguous memory.
              It is build upon std::vector.
 