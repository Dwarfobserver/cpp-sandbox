
#pragma once

#include <unordered_set>

namespace sc {

    /// Tracks objects and creates them with it's factories.
    class arc_garbage_collector;

    /// Owner pointer to a object tracked by a garbage collector.
    template <class T>
    class gc_ptr;

    namespace detail {
        /// Type-erase gc_node to use and delete it uniformly.
        struct gc_node_header;

        /// Handle a value allocated, with it's reference counter.
        template <class T, class Allocator>
        struct gc_node;

        /// Create instances of T with the given allocator.
        template <class T, class Allocator>
        struct gc_factory;

        /// Type-erased function signature which deletes nodes.
        using gc_node_deleter_t = void (*) (gc_node_header*);
    }

    class arc_garbage_collector {
        template <class T, class Allocator>
        friend class detail::gc_factory;

        using set_type = std::unordered_set<detail::gc_node_header*>;
    public:
        arc_garbage_collector() = default;

        template <class T, class Allocator = std::allocator<T>>
        detail::gc_factory<T, Allocator> factory(Allocator const& allocator = Allocator());

        void collect();

        int references_count() const;
    private:
        set_type nodes_;
    };

    namespace detail {
        struct gc_node_header {
            uint32_t refs_;
            gc_node_deleter_t deleter_;

            inline explicit gc_node_header(gc_node_deleter_t deleter);

            inline void increment();
            inline void decrement();

            template <class T>
            T* get();
        };

        template <class T, class Allocator>
        struct gc_node {
            gc_node_header header_;
            T val_;
            Allocator allocator_;

            static void deleter(gc_node_header* node);

            template <class...Args>
            explicit gc_node(Allocator& allocator, Args&&...args);
        };

        template <class T, class Allocator>
        struct gc_factory {
            using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
            using allocator_traits = std::allocator_traits<allocator_type>;

            explicit gc_factory(::sc::arc_garbage_collector& gc, Allocator const& allocator);

            template <class...Args>
            gc_ptr<T> make(Args&&...args);
        private:
            ::sc::arc_garbage_collector& gc_;
            allocator_type allocator_;
        };
    }

    // ______________
    // Implementation

    // arc_garbage_collector

    template <class T, class Allocator>
    detail::gc_factory<T, Allocator> arc_garbage_collector::factory(Allocator const& allocator) {
        return detail::gc_factory<T, Allocator>{ *this, allocator };
    }

    void arc_garbage_collector::collect() {
        std::vector<set_type::const_iterator> iterators;
        auto begin = nodes_.begin();
        while (begin != nodes_.end()) {
            if ((*begin)->refs_ == 0) iterators.push_back(begin);
            ++begin;
        }
        for (auto& it : iterators) {
            (*it)->deleter_(*it);
            nodes_.erase(it);
        }
    }

    int arc_garbage_collector::references_count() const {
        return static_cast<int>(nodes_.size());
    }

    // detail::gc_node_header

    namespace detail {
        gc_node_header::gc_node_header(gc_node_deleter_t deleter) :
                refs_(0),
                deleter_(deleter)
        {}

        void gc_node_header::increment() { ++refs_; }
        void gc_node_header::decrement() { --refs_; }

        template <class T>
        T* gc_node_header::get() { return reinterpret_cast<T*>(this + 1); }
    }

    // detail::gc_node

    namespace detail {
        template <class T, class Allocator>
        void gc_node<T, Allocator>::deleter(gc_node_header* node) {
            auto pNode = reinterpret_cast<gc_node*>(node);
            pNode->val_.~T();
            std::allocator_traits<Allocator>::destroy(pNode->allocator_, pNode);
        }

        template <class T, class Allocator>
        template <class...Args>
        gc_node<T, Allocator>::gc_node(Allocator& allocator, Args&&...args) :
                header_(deleter),
                allocator_(allocator),
                val_(std::forward<Args>(args)...)
        {}
    }

    // detail::gc_factory

    namespace detail {
        template <class T, class Allocator>
        gc_factory<T, Allocator>::gc_factory(::sc::arc_garbage_collector& gc, Allocator const& allocator) :
                gc_(gc),
                allocator_(allocator)
        {}

        template <class T, class Allocator>
        template <class...Args>
        gc_ptr<T> gc_factory<T, Allocator>::make(Args&&...args) {
            auto ptr = allocator_traits::allocate(allocator_, 1);
            auto pNode = reinterpret_cast<gc_node<T, Allocator>*>(ptr);
            auto pHeader = reinterpret_cast<gc_node_header*>(ptr);

            new (pNode) gc_node<T, allocator_type>(allocator_, std::forward<Args>(args)...);
            gc_.nodes_.insert(pHeader);
            return gc_ptr<T>{ *pHeader };
        }
    }

    template <class T>
    class gc_ptr {
        friend class arc_garbage_collector;
    public:
        explicit gc_ptr(detail::gc_node_header& ptr) : ptr_{ &ptr } { ptr_->increment(); }

        gc_ptr() : ptr_{} {}
        ~gc_ptr() noexcept { decrement(); }

        gc_ptr(gc_ptr&& rhs) noexcept : ptr_{rhs.ptr_} { rhs.ptr_ = nullptr; }
        gc_ptr(gc_ptr const& rhs) : ptr_{rhs.ptr_} { increment(); }

        gc_ptr& operator=(gc_ptr&& rhs) noexcept {
            decrement();
            ptr_ = rhs.ptr_;
            rhs.ptr_ = nullptr;
        }
        gc_ptr& operator=(gc_ptr const& rhs) {
            decrement();
            ptr_ = rhs.ptr_;
            increment();
        }

        T* operator->()          { return  ptr_->get<T>(); }
        T& operator*()           { return *ptr_->get<T>(); }
        explicit operator bool() { return  ptr_ != nullptr; }
    private:
        void increment() { if (ptr_) ptr_->increment(); }
        void decrement() { if (ptr_) ptr_->decrement(); }

        detail::gc_node_header* ptr_;
    };

}
