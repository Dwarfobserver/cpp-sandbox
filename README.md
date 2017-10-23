
# cpp-sandbox

This is a repository containing some~~reusable~~C++ code I developed.
It uses some C++17 features (some others are missing from mingw). 

Tests are run with Catch and can be used as a poor documentation.

 - utils : Few useful or funny things :
      - An operator used to safely chain calls of functions which can return an optional.
      - An aligned allocator, used to fill cache lines where we want to store our false sharing sensible data.

 - spsc_queue : Wait-free single producer & single consumer queue.
                This class do not check for overflow (it have a good chance to throw in debug mode).

 - mpsc_queue : Lock-free multiple producer & single (wait-free) consumer.
                This class is safe from overflow, but can then block producers.

 - fluent_collections : Functional-style wrapper of "collections" :
                        Template classes with functions begin(), end() and insert(iterator) that behave gently.

 - aligned_array : Little structure which allocate & free an aligned array (useful for cache lines).
 
