
#pragma once

#include <utils.hpp>

#include <atomic>
#include <iterator>
#include <bits/allocator.h>


namespace sc {

    template <class T, template <class> class alloc_t = std::allocator>
    class fwd_list {
        class node_t;
    public:
        class unsafe_iterator;
        class iterator;

        iterator begin() noexcept;
        unsafe_iterator ubegin() noexcept;
        unsafe_iterator end() const noexcept;

        fwd_list() noexcept;
        ~fwd_list() noexcept;

        bool empty() const noexcept;

        template <class...Args>
        void emplace_front(Args&&...args);
        template <class...Args>
        void emplace_after(iterator it, Args &&...args);

        void push_front(T&& moved)              { return emplace_front(std::move(moved)); }
        void push_after(iterator it, T &&moved) { return emplace_after(it, std::move(moved)); }
        void push_front(T const& clone)              { return emplace_front(const_cast<T&>(clone)); }
        void push_after(iterator it, T const &clone) { return emplace_after(it, const_cast<T&>(clone)); }

        bool erase_front() noexcept;
        bool erase_after(iterator it) noexcept;

        fwd_list(fwd_list const&) = delete;
        fwd_list& operator=(fwd_list const&) = delete;
        fwd_list(fwd_list &&) = delete;
        fwd_list& operator=(fwd_list &&) = delete;
    private:
        std::atomic<node_t*> readHead;
        std::atomic<node_t*> writeHead;
    public:
        class unsafe_iterator {
            friend class fwd_list;

            node_t* pNode;

            unsafe_iterator() noexcept : pNode(nullptr) {}
            explicit unsafe_iterator(node_t* pNode) noexcept : pNode(pNode) {}
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            using unsafe_iterator_category = std::forward_iterator_tag;

            // Marked nodiscard in case the user thought it modified the pointer
            [[nodiscard]] iterator make_safe() const noexcept;

            ~unsafe_iterator() noexcept = default;
            unsafe_iterator(unsafe_iterator const& clone) noexcept = default;
            unsafe_iterator& operator=(unsafe_iterator const& clone) noexcept = default;
            unsafe_iterator(unsafe_iterator && old) noexcept = default;
            unsafe_iterator& operator=(unsafe_iterator && old) noexcept = default;

            T& operator*() noexcept { return pNode->val; }
            T* operator->() noexcept { return &pNode->val; }

            unsafe_iterator& operator++() noexcept {
                pNode = pNode->readNext.load();
                return *this;
            }
            unsafe_iterator operator++(int) noexcept {
                auto clone = unsafe_iterator(*this);
                pNode = pNode->readNext.load();
                return clone;
            }

            bool operator==(const unsafe_iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const unsafe_iterator& rhs) const noexcept { return pNode != rhs.pNode; }
            bool operator==(const iterator& rhs) const noexcept;
            bool operator!=(const iterator& rhs) const noexcept;
        };
        class iterator {
            friend class fwd_list;

            node_t* pNode;

            iterator() noexcept : pNode(nullptr) {}
            explicit iterator(node_t* pNode) noexcept : pNode(pNode) {
                if (pNode != nullptr) pNode->addRef();
            }
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::forward_iterator_tag;

            // Marked nodiscard in case the user thought it modified the pointer
            [[nodiscard]] unsafe_iterator make_unsafe() const noexcept {
                return unsafe_iterator(pNode);
            }

            ~iterator() noexcept {
                if (pNode != nullptr) pNode->removeRef();
            }

            iterator(iterator const& clone) noexcept : pNode(clone.pNode) {
                if (pNode != nullptr) pNode->addRef();
            }
            iterator& operator=(iterator const& clone) noexcept {
                if (pNode != nullptr) pNode->removeRef();

                pNode = clone.pNode;
                if (pNode != nullptr) pNode->addRef();
                return *this;
            }

            iterator(iterator && old) noexcept : pNode(old.pNode) {
                old.pNode = nullptr;
            }
            iterator& operator=(iterator && old) noexcept {
                if (pNode != nullptr) pNode->removeRef();

                pNode = old.pNode;
                old.pNode = nullptr;
                return *this;
            }

            T& operator*() noexcept { return pNode->val; }
            T* operator->() noexcept { return &pNode->val; }

            iterator& operator++() noexcept {
                if (pNode != nullptr) {
                    const auto pNext = pNode->readNext.load();
                    if (pNext != nullptr) pNext->addRef();
                    pNode->removeRef();
                    pNode = pNext;
                }
                return *this;
            }
            iterator operator++(int) noexcept {
                auto clone = iterator(*this);
                if (pNode != nullptr) {
                    const auto pNext = pNode->readNext.load();
                    if (pNext != nullptr) pNext->addRef();
                    pNode->removeRef();
                    pNode = pNext;
                }
                return clone;
            }

            bool operator==(const iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const iterator& rhs) const noexcept { return pNode != rhs.pNode; }
            bool operator==(const unsafe_iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const unsafe_iterator& rhs) const noexcept { return pNode != rhs.pNode; }
        };
    private:
        template <class...Args>
        node_t* construct(Args&&...args);

        struct bind_t {
            std::atomic<node_t*> next;
            std::atomic<node_t*> tmpNext;

        private:
            std::atomic_int refCount;
        };
        struct node_t {
            alignas(SC_CACHE_LINE_SIZE)
            T val;
            alignas(SC_CACHE_LINE_SIZE)
            std::atomic_int refCount;
            std::atomic<node_t*> readNext;
            std::atomic<node_t*> writeNext;

            void addRef() { ++refCount; }
            // Note : Never use the node after removeRef()
            void removeRef() {
                if (--refCount == 0) {
                    val.~T();
                    alloc_t<node_t>().deallocate(this, 1);
                }
            }
        };
    };

    // ______________
    // Implementation

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::iterator fwd_list<T, alloc_t>::begin() noexcept {
        return iterator(readHead.load());
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::unsafe_iterator fwd_list<T, alloc_t>::ubegin() noexcept {
        return unsafe_iterator(readHead.load());
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::unsafe_iterator fwd_list<T, alloc_t>::end() const noexcept {
        return unsafe_iterator();
    }

    template <class T, template <class> class alloc_t>
    fwd_list<T, alloc_t>::fwd_list() noexcept :
        readHead(nullptr), writeHead(nullptr)
    {}

    template <class T, template <class> class alloc_t>
    fwd_list<T, alloc_t>::~fwd_list() noexcept {
        auto pNode = readHead.load();
        while (pNode != nullptr) {
            auto pNext = pNode->readNext.load();
            pNode->removeRef();
            pNode = pNext;
        }
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::empty() const noexcept {
        return readHead.load(std::memory_order_consume) == nullptr;
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    void fwd_list<T, alloc_t>::emplace_front(Args &&... args) {
        node_t* pOld;
        auto pNew = construct(std::forward<T>(args)...);
        do {
            pOld = readHead.load();
            pNew->readNext.store(pOld);
            pNew->writeNext.store(pOld);
        } while (!writeHead.compare_exchange_weak(pOld, pNew));

        readHead.store(pNew);
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    void fwd_list<T, alloc_t>::emplace_after(iterator it, Args &&... args) {
        node_t* pOld;
        auto pNew = construct(std::forward<T>(args)...);
        do {
            pOld = it.pNode->readNext.load();
            pNew->readNext.store(pOld);
            pNew->writeNext.store(pOld);
        } while (!it.pNode->writeNext.compare_exchange_weak(pOld, pNew));

        it.pNode->readNext.store(pNew);
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::erase_front() noexcept {
        node_t* pCurrent;
        node_t* pNext;
        do {
            pCurrent = readHead.load();
            if (pCurrent == nullptr) return false;
            pNext = pCurrent->readNext.load();
        } while (!writeHead.compare_exchange_weak(pCurrent, pNext));

        readHead.store(pNext);
        pCurrent->removeRef();
        return true;
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::erase_after(iterator it) noexcept {
        node_t* pCurrent;
        node_t* pNext;
        do {
            pCurrent = it.pNode->readNext.load();
            if (pCurrent == nullptr) return false;
            pNext = pCurrent->readNext.load();
        } while (!it.pNode->writeNext.compare_exchange_weak(pCurrent, pNext));

        it.pNode->readNext.store(pNext);
        pCurrent->removeRef();
        return true;
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    typename fwd_list<T, alloc_t>::node_t *fwd_list<T, alloc_t>::construct(Args &&... args) {
        node_t* pNode = alloc_t<node_t>().allocate(1);
        new (&pNode->val) T(std::forward<T>(args)...);
        return pNode;
    }

    // Iterators functions (which couldn't be written at declaration)

    template <class T, template <class> class alloc_t>
    [[nodiscard]] typename fwd_list<T, alloc_t>::iterator fwd_list<T, alloc_t>::unsafe_iterator::make_safe() const noexcept {
        return iterator(pNode);
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::unsafe_iterator::operator==(const iterator &rhs) const noexcept {
        return pNode == rhs.pNode;
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::unsafe_iterator::operator!=(const iterator &rhs) const noexcept {
        return pNode != rhs.pNode;
    }

} // End of ::sc
