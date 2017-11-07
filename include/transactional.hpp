
#pragma once

#include <bits/allocator.h>
#include <atomic>
#include <stdexcept>


namespace sc {

    template <class T, template <class> class Allocator = std::allocator>
    class transactional {
        class node_t;
    public:
        class handle_t;

        transactional() noexcept;
        ~transactional() noexcept;

        bool empty() const noexcept;

        template <class...Args>
        handle_t create(Args&&...args);

        template <class F>
        void modify(handle_t& handle, F&& modifier); // F has operator()(T&)

        void update(handle_t& handle);

        void clear() noexcept; // Not thread-safe with another clear

        class handle_t {
            friend class transactional;
        public:
            handle_t() noexcept;
            ~handle_t() noexcept;
            handle_t(handle_t&& moved) noexcept;
            handle_t(handle_t const& clone) noexcept;
            handle_t& operator=(handle_t&& moved) noexcept;
            handle_t& operator=(handle_t const& clone) noexcept;

            operator bool() const noexcept { return node != nullptr; }
            T const& operator*() const noexcept { return node->val; }
            T const* operator->() const noexcept { return &node->val; }
        private:
            explicit handle_t(node_t* node) noexcept : node{node} {}

            node_t* node;
        };
    private:
        static bool clear_rec(node_t* node) noexcept;

        struct node_t {
            T val;
            std::atomic<int> refCount;
            std::atomic<node_t*> next;
        };

        std::atomic<node_t*> head;
    };

    // ______________
    // Implementation

    // Transactional

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::transactional() noexcept :
            head{nullptr}
    {}

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::~transactional() noexcept {
        node_t* node;
        auto next = head.load();
        while (next != nullptr) {
            node = next;
            next = node->next.load();
            node->val.~T();
            Allocator<node_t>().deallocate(node, 1);
        }
    }

    template <class T, template <class> class Allocator>
    bool transactional<T, Allocator>::empty() const noexcept {
        return head.load() == nullptr;
    }

    template <class T, template <class> class Allocator> template<class...Args>
    typename transactional<T, Allocator>::handle_t
    transactional<T, Allocator>::create(Args&&...args) {
#ifndef NDEBUG
        if (!empty()) throw std::logic_error{"Tried to create an already created transactional value."};
#endif
        node_t* pNode = Allocator<node_t>().allocate(1);
        new (&pNode->val) T(std::forward<Args>(args)...);
        pNode->next.store(nullptr);
        pNode->refCount.store(1);
        head.store(pNode);

        return handle_t{pNode};
    }

    template <class T, template <class> class Allocator> template<class F>
    void transactional<T, Allocator>::modify(handle_t &handle, F &&modifier) {
        node_t* pNew = Allocator<node_t>().allocate(1);
        pNew->refCount.store(1);

        auto pOld = head.load();
        // Try to commit changes
        new (&pNew->val) T(pOld->val);
        modifier(pNew->val);
        while (!head.compare_exchange_weak(pOld, pNew)) {
            pNew->val.~T();
            new (&pNew->val) T(pOld->val);
            modifier(pNew->val);
        }
        handle.node->refCount.fetch_sub(1);
        handle.node = pNew;
    }

    template <class T, template <class> class Allocator>
    void transactional<T, Allocator>::update(handle_t &handle) {
        auto pOld = handle.node;
        handle.node = head.load();
        handle.node->refCount.fetch_add(1);
        pOld->refCount.fetch_sub(1);
    }

    template <class T, template <class> class Allocator>
    void transactional<T, Allocator>::clear() noexcept {
        if (clear_rec(head.load())) {
            head.store(nullptr);
        }
    }

    template <class T, template <class> class Allocator>
    bool transactional<T, Allocator>::clear_rec(node_t* node) noexcept {
        if (node == nullptr)
            return true;

        if (!clear_rec(node->next))
            return false;

        if (node->refCount.load() == 0) {
            node->val.~T();
            Allocator<node_t>().deallocate(node, 1);
            return true;
        }
        else {
            node->next.store(nullptr);
            return false;
        }
    }

    // Handle

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::handle_t() noexcept :
            node{nullptr}
    {}

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::~handle_t() noexcept {
        if (node != nullptr) node->refCount.fetch_sub(1);
    }

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(handle_t&& moved) noexcept :
            node{node} {
        moved.node = nullptr;
    }

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(handle_t const& clone) noexcept :
            node{node} {
        node->refCount.fetch_add(1);
    }

    template <class T, template <class> class Allocator>
    typename transactional<T, Allocator>::handle_t&
    transactional<T, Allocator>::handle_t::operator=(handle_t&& moved) noexcept {
        if (this == &moved) return *this;
        if (node != nullptr) node->refCount.fetch_sub(1);
        node = moved.node;
        moved.node = nullptr;
        return *this;
    }

    template <class T, template <class> class Allocator>
    typename transactional<T, Allocator>::handle_t&
    transactional<T, Allocator>::handle_t::operator=(handle_t const& clone) noexcept {
        if (this == &clone) return *this;
        if (clone.node != nullptr) clone.node->refCount.fetch_add(1);
        if (node != nullptr) node->refCount.fetch_sub(1);
        node = clone.node;
        return *this;
    }

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::operator bool() const noexcept {
        return node != nullptr;
    }

    template <class T, template <class> class Allocator>
    T const& transactional<T, Allocator>::handle_t::operator*() const noexcept {
        return node->val;
    }

    template <class T, template <class> class Allocator>
    T const* transactional<T, Allocator>::handle_t::operator->() const noexcept {
        return &node->val;
    }

    template <class T, template <class> class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(node_t* node) noexcept :
            node{node}
    {}

}
