
#pragma once

#include <bits/allocator.h>
#include <atomic>
#include <stdexcept>


namespace sc {

    template <class T, class Allocator = std::allocator<T>>
    class transactional {
        class node_t;
        using node_allocator_t = typename std::allocator_traits<Allocator>::template rebind_alloc<node_t>;
    public:
        class handle_t;

        explicit transactional(Allocator const& allocator = Allocator()) noexcept;
        ~transactional() noexcept;

        bool empty() const noexcept;

        template <class...Args>
        handle_t create(Args&&...args);

        template <class F>
        void modify(handle_t& handle, F&& modifier); // F has operator()(T&)

        void update(handle_t& handle);

        void clear() noexcept; // Not thread-safe with another clear

        class handle_t {
            friend class transactional<T, Allocator>;
        public:
            handle_t() noexcept;
            ~handle_t() noexcept;
            handle_t(handle_t&& moved) noexcept;
            handle_t(handle_t const& clone) noexcept;
            handle_t& operator=(handle_t&& moved) noexcept;
            handle_t& operator=(handle_t const& clone) noexcept;

            operator bool() const noexcept;
            T const& operator*() const noexcept;
            T const* operator->() const noexcept;
        private:
            explicit handle_t(node_t* node) noexcept;

            node_t* node_;
        };
    private:
        bool clear_rec(node_t* node) noexcept;

        struct node_t {
            T val_;
            std::atomic<int> refCount_;
            std::atomic<node_t*> next_;
        };

        std::atomic<node_t*> head_;
        node_allocator_t allocator_;
    };

    // ______________
    // Implementation

    // Transactional

    template <class T, class Allocator>
    transactional<T, Allocator>::transactional(Allocator const& allocator) noexcept :
            head_{nullptr},
            allocator_(allocator)
    {}

    template <class T, class Allocator>
    transactional<T, Allocator>::~transactional() noexcept {
        node_t* node;
        auto next = head_.load();
        while (next != nullptr) {
            node = next;
            next = node->next_.load();
            node->val_.~T();
            std::allocator_traits<node_allocator_t>::deallocate(allocator_, node, 1);
        }
    }

    template <class T, class Allocator>
    bool transactional<T, Allocator>::empty() const noexcept {
        return head_.load() == nullptr;
    }

    template <class T, class Allocator> template<class...Args>
    typename transactional<T, Allocator>::handle_t
    transactional<T, Allocator>::create(Args&&...args) {
#ifndef NDEBUG
        if (!empty()) throw std::logic_error{"Tried to create an already created transactional value."};
#endif
        node_t* pNode = std::allocator_traits<node_allocator_t>::allocate(allocator_, 1);
        new (&pNode->val_) T(std::forward<Args>(args)...);
        pNode->next_.store(nullptr);
        pNode->refCount_.store(1);
        head_.store(pNode);

        return handle_t{pNode};
    }

    template <class T, class Allocator> template<class F>
    void transactional<T, Allocator>::modify(handle_t &handle, F &&modifier) {
        node_t* pNew = std::allocator_traits<node_allocator_t>::allocate(allocator_, 1);
        pNew->refCount_.store(1);

        auto pOld = head_.load();
        // Try to commit changes
        pNew->next_.store(pOld);
        new (&pNew->val_) T(pOld->val_);
        modifier(pNew->val_);
        while (!head_.compare_exchange_weak(pOld, pNew)) {
            pNew->val_.~T();
            pNew->next_.store(pOld);
            new (&pNew->val_) T(pOld->val_);
            modifier(pNew->val_);
        }
        handle.node_->refCount_.fetch_sub(1);
        handle.node_ = pNew;
    }

    template <class T, class Allocator>
    void transactional<T, Allocator>::update(handle_t &handle) {
        auto pOld = handle.node_;
        handle.node_ = head_.load();
        handle.node_->refCount_.fetch_add(1);
        pOld->refCount_.fetch_sub(1);
    }

    template <class T, class Allocator>
    void transactional<T, Allocator>::clear() noexcept {
        if (clear_rec(head_.load())) {
            head_.store(nullptr);
        }
    }

    template <class T, class Allocator>
    bool transactional<T, Allocator>::clear_rec(node_t* node) noexcept {
        if (node == nullptr)
            return true;

        if (!clear_rec(node->next_))
            return false;

        if (node->refCount_.load() == 0) {
            node->val_.~T();
            std::allocator_traits<node_allocator_t>::deallocate(allocator_, node, 1);
            return true;
        }
        else {
            node->next_.store(nullptr);
            return false;
        }
    }

    // Handle

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::handle_t() noexcept :
            node_{nullptr}
    {}

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::~handle_t() noexcept {
        if (node_ != nullptr) node_->refCount_.fetch_sub(1);
    }

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(handle_t&& moved) noexcept :
            node_{moved.node_}
    {
        moved.node_ = nullptr;
    }

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(handle_t const& clone) noexcept :
            node_{clone.node_}
    {
        node_->refCount_.fetch_add(1);
    }

    template <class T, class Allocator>
    typename transactional<T, Allocator>::handle_t&
    transactional<T, Allocator>::handle_t::operator=(handle_t&& moved) noexcept {
        if (this == &moved) return *this;
        if (node_ != nullptr) node_->refCount_.fetch_sub(1);
        node_ = moved.node_;
        moved.node_ = nullptr;
        return *this;
    }

    template <class T, class Allocator>
    typename transactional<T, Allocator>::handle_t&
    transactional<T, Allocator>::handle_t::operator=(handle_t const& clone) noexcept {
        if (this == &clone) return *this;
        if (clone.node_ != nullptr) clone.node_->refCount_.fetch_add(1);
        if (node_ != nullptr) node_->refCount_.fetch_sub(1);
        node_ = clone.node_;
        return *this;
    }

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::operator bool() const noexcept {
        return node_ != nullptr;
    }

    template <class T, class Allocator>
    T const& transactional<T, Allocator>::handle_t::operator*() const noexcept {
        return node_->val_;
    }

    template <class T, class Allocator>
    T const* transactional<T, Allocator>::handle_t::operator->() const noexcept {
        return &node_->val_;
    }

    template <class T, class Allocator>
    transactional<T, Allocator>::handle_t::handle_t(node_t* node) noexcept :
            node_{node}
    {}

}
