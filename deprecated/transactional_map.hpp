
#pragma once

#include <bits/allocator.h>
#include <bits/unique_ptr.h>
#include <stdexcept>
#include <atomic>
#include <mutex>

/*
namespace sc { // TODO Iterators, array->map : marks for remove, swaps, id list, ...

    template<class T, template<class> class alloc_t = std::allocator>
    class transactional_map {
        using lock_t = std::lock_guard<std::mutex>;
        class node_t;
        class entry_t;
    public:
        class handle_t;

        explicit transactional_map(int capacity);
        ~transactional_map() noexcept;

        int size() const noexcept;

        template <class...Args>
        handle_t create(Args &&...args);

        void setVisible(handle_t const &handle, bool visible) noexcept;

        template <class F>
        handle_t modify(handle_t const& handle, F&& f);

        handle_t update(handle_t const& handle) noexcept;

        bool clean() noexcept;
    private:
        std::atomic<int> _size;
        int capacity;
        entry_t* entries;
        std::atomic<bool> cleanFlag;
    public:
        class handle_t {
            friend class transactional_map;
            friend class node_t;
        public:
            operator bool() const noexcept;
            T const& operator*() const noexcept;
            T const* operator->() const noexcept;

            void reset() noexcept;
            handle_t() noexcept;
            ~handle_t() noexcept;

            handle_t(handle_t const& clone) noexcept;
            handle_t& operator=(handle_t const& clone) noexcept;

            handle_t(handle_t && moved) noexcept;
            handle_t& operator=(handle_t && moved) noexcept;
        private:
            node_t* node;
            explicit handle_t(node_t* node) noexcept : node(node) {}
        };
        class iterator {
        public:

        };
    private:
        class node_t {
        public:
            T val;
            int id;
            std::atomic<node_t*> next;
            std::atomic<int> refCount;

            template <class...Args>
            explicit node_t(int id, Args &&...args) :
                    val(std::forward<Args>(args)...),
                    id(id),
                    next(nullptr),
                    refCount(0)
            {};

            bool clean() {
                auto pNext = next.load();
                if (pNext != nullptr) {
                    if(pNext->clean()) {
                        next.store(nullptr);
                    }
                    else return false;
                }

                auto refs = refCount.load();
                if (refs > 0)
                    return false;

                val.~T();
                alloc_t<node_t>().deallocate(this, 1);
                return true;
            }
        };
        struct entry_t {
            std::atomic<node_t*> node;
            std::atomic<bool> visible;

            entry_t() noexcept : node(nullptr), visible(false) {}
        };
    };

    // ______________
    // Implementation

    template<class T, template<class> class alloc_t>
    transactional_map<T, alloc_t>::transactional_map(int capacity) :
            capacity(capacity),
            _size(0),
            cleanFlag(false),
            entries(nullptr)
    {
        if (capacity < 1) throw std::invalid_argument{"transactional_map capacity must be superior to 0."};

        entries = alloc_t<entry_t>().allocate(capacity);
        for (int i = 0; i < capacity; ++i) {
            new (entries + i) entry_t();
        }
    }

    template<class T, template<class> class alloc_t>
    int transactional_map<T, alloc_t>::size() const noexcept {
        return _size.load();
    }

    template<class T, template<class> class alloc_t> template<class...Args>
    typename transactional_map<T, alloc_t>::handle_t transactional_map<T, alloc_t>::create(Args &&... args) {
        int id = _size.fetch_add(1);

        if (id >= capacity) {
            _size.fetch_sub(1);
            throw std::runtime_error{"transactional_map has been overflowed."};
        }
        // Create node
        node_t* pNew = alloc_t<node_t>().allocate(1);
        new (pNew) node_t(id, std::forward<Args>(args)...);
        pNew->refCount.store(1);
        // Set entry
        entries[id].node.store(pNew);
        entries[id].visible.store(true);

        return handle_t(pNew);
    }

    template<class T, template<class> class alloc_t>
    void transactional_map<T, alloc_t>::setVisible(handle_t const &handle, bool visible) noexcept {
        if (!handle) return;

        // Valid handle means valid node pointer
        entries[handle.node->id].visible.store(visible);
    }
}
*/