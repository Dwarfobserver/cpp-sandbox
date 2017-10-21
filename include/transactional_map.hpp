
#pragma once

#include <bits/allocator.h>
#include <bits/unique_ptr.h>
#include <stdexcept>
#include <atomic>
#include <mutex>


namespace sc { // TODO Iterators, array->map : marks for remove, swaps, id list, ...

    template<class T, template<class> class alloc_t = std::allocator>
    class transactional_map {
        using lock_t = std::lock_guard<std::mutex>;
        class bind_t;
        class node_t;
    public:
        class handle_t;

        explicit transactional_map(int capacity);
        ~transactional_map() noexcept;

        int size() const noexcept;

        template <class...Args>
        handle_t create(Args &&...args);

        template <class F>
        handle_t modify(handle_t const& handle, F&& f);

        handle_t update(handle_t const& handle) noexcept;

        bool clean() noexcept;
    private:
        std::atomic<int> _size;
        int capacity;
        bind_t* binds;
        std::atomic<bool> cleanFlag;
    public:
        class handle_t {
            friend class transactional_map;
            friend class bind_t;
        public:
            operator bool() const noexcept { return bind != nullptr; }
            T const& operator*() const noexcept { return *val; }
            T const* operator->() const noexcept { return val; }

            void reset() noexcept {
                if (*this) bind->releaseNode();
                bind = nullptr;
                val = nullptr;
            }
            handle_t() noexcept :
                    bind(nullptr), val(nullptr)
            {}
            ~handle_t() noexcept {
                if (*this) bind->releaseNode();
            }

            handle_t(handle_t const& clone) noexcept :
                    bind(clone.bind), val(clone.val)
            {
                if (*this) bind->tryLockNode();
            }
            handle_t& operator=(handle_t const& clone) noexcept {
                auto oldBind = bind;
                bind = clone.bind;
                val = clone.val;
                if (*this) bind->tryLockNode();
                if (oldBind != nullptr) oldBind->releaseNode();
            }

            handle_t(handle_t && moved) noexcept :
                    bind(moved.bind), val(moved.val)
            {
                moved.bind = nullptr;
                moved.val = nullptr;
            }
            handle_t& operator=(handle_t && moved) noexcept {
                if (*this) bind->releaseNode();
                bind = moved.bind;
                val = moved.val;
                moved.bind = nullptr;
                moved.val = nullptr;
            }
        private:
            bind_t* bind;
            T* val;

            explicit handle_t(bind_t* bind) noexcept :
                    bind(bind)
            {
                val = (!*this)
                      ? nullptr
                      : &bind->node.load()->val;
            }
        };
    private:
        class bind_t {
        public:
            int id;
            std::atomic<node_t*> node;
            std::atomic<int> refCount;

            explicit bind_t(int id) : id(id), node(0), refCount(0) {}

            bool tryLockNode() { // TODO Verify : cmp_xch or just increment ?
                // Increment the reference counter if he is valid (count >= 0)
                auto count = refCount.load(std::memory_order_acquire);
                do {
                    if (count == -1) return false;
                } while (refCount.compare_exchange_weak(count, count + 1));

                return true;
            }
            void releaseNode() {
                refCount.fetch_sub(1);
            }
            bool recursiveClean() {
                auto pNode = node.load(std::memory_order_acquire);

                if (pNode == nullptr)
                    return true;

                // Stops if recursion doesn't delete
                if (!pNode->bind.recursiveClean())
                    return false;

                return tryDestroyNode();
            }
        private:
            bool tryDestroyNode() {
                // Check if there is no references
                int unusedRefValue = 0;
                if (!refCount.compare_exchange_strong(unusedRefValue, -1))
                    return false;

                // Destroy node, don't point at next because they must be removed
                const auto pNode = node.load(std::memory_order_acquire);
                node.store(nullptr, std::memory_order_relaxed);
                pNode->val.~T();
                alloc_t<node_t>().deallocate(pNode, 1);

                return true;
            }
        };
        struct node_t {
            bind_t bind;
            T val;
        };
    };

    // ______________
    // Implementation

    template<class T, template<class> class alloc_t>
    transactional_map<T, alloc_t>::transactional_map(int capacity) :
            capacity(capacity),
            _size(0),
            cleanFlag(false)
    {
        if (capacity < 1) throw std::invalid_argument{"transactional_map capacity must be superior to 0."};

        // Create bind array
        binds = alloc_t<bind_t>().allocate(capacity);
        for (int i = 0; i < capacity; ++i) {
            new (binds + i) bind_t(i);
        }
    }

    template<class T, template<class> class alloc_t>
    transactional_map<T, alloc_t>::~transactional_map() noexcept { // TODO Force destructions (handles can be destroyed after)
        while (!clean());
        alloc_t<bind_t>().deallocate(binds, capacity);
    }

    template<class T, template<class> class alloc_t>
    int transactional_map<T, alloc_t>::size() const noexcept {
        return _size.load(std::memory_order_consume);
    }

    template<class T, template<class> class alloc_t> template<class...Args>
    typename transactional_map<T, alloc_t>::handle_t
    transactional_map<T, alloc_t>::create(Args &&... args) {
        // Get id
        const auto id = _size.fetch_add(1, std::memory_order_acquire);
        if (id >= capacity) {
            _size.fetch_sub(1, std::memory_order_relaxed);
            throw std::runtime_error{"transactional_map have been overflowed."};
        }
        // New node
        node_t* const pNew = alloc_t<node_t>().allocate(1);
        new (&pNew->bind) bind_t(id);
        new (&pNew->val) T(std::forward<T>(args)...);
        pNew->bind.tryLockNode();
        handle_t handle(&pNew->bind);

        // Try to commit
        node_t* pOld = binds[id].node.load();
        do {
            pNew->bind.node.store(pOld);
        } while (binds[id].node.compare_exchange_weak(pOld, pNew));

        return handle;
    }

    template<class T, template<class> class alloc_t> template<class F>
    typename transactional_map<T, alloc_t>::handle_t transactional_map<T, alloc_t>::modify(handle_t const& handle, F &&f) {
        if (!handle)
            return {};

        const auto id = handle.bind->id;

        // New node
        node_t* const pNew = alloc_t<node_t>().allocate(1);
        new (&pNew->bind) bind_t(id);
        pNew->bind.tryLockNode();
        handle_t newHandle(&pNew->bind);

        // Try to commit
        auto pOld = binds[id].node.load();
        pNew->bind.node.store(pOld);
        new (&pNew->val) T(pOld->val);
        f(pNew->val);
        while (!binds[id].node.compare_exchange_weak(pOld, pNew)) {
            pNew->val.~T();
            pNew->bind.node.store(pOld);
            new (&pNew->val) T(pOld->val);
            f(pNew->val);
        };

        return newHandle;
    }

    template<class T, template<class> class alloc_t>
    typename transactional_map<T, alloc_t>::handle_t transactional_map<T, alloc_t>::update(handle_t const& handle) noexcept {
        if (!handle)
            return {};

        auto pBind = &binds[handle.bind->id];
        // No valid value
        if (pBind->node.load() == nullptr)
            return {};
        return handle_t(pBind);
    }

    template<class T, template<class> class alloc_t>
    bool transactional_map<T, alloc_t>::clean() noexcept {
        // Critical section
        bool flagAvailableValue = false;
        if (!cleanFlag.compare_exchange_strong(flagAvailableValue, true, std::memory_order_acquire)) return false;

        for (int i = 0; i < capacity; ++i) {
            binds[i].recursiveClean();
        }

        // End of critical section
        cleanFlag.store(false, std::memory_order_release);
        return true;
    }

}
