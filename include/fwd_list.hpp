
#pragma once

#include <utils.hpp>

#include <bits/allocator.h>
#include <atomic>
#include <iterator>


namespace sc {

    template <class T, template <class> class alloc_t = std::allocator>
    class fwd_list {
        struct node_t {
            friend class iterator;
            friend class const_iterator;

            T val;
            std::atomic<node_t*> readNext;
            alignas(SC_CACHE_LINE_SIZE) std::atomic<node_t*> writeNext;
        };
    public:
        class const_iterator;
        class iterator {
            friend class fwd_list;

            node_t* pNode;

            explicit iterator(node_t* pNode) noexcept : pNode(pNode) {}
        public:
            using value_type = T;
            using difference_type = ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::forward_iterator_tag;

            iterator() noexcept : pNode(nullptr) {}
            ~iterator() noexcept = default;

            iterator(iterator const& clone) noexcept : pNode(clone.pNode) {}
            iterator& operator=(iterator const& clone) noexcept {
                pNode = clone.pNode;
                return *this;
            }

            T& operator*() noexcept { return pNode->val; }
            T* operator->() noexcept { return &pNode->val; }

            iterator& operator++() noexcept {
                pNode = pNode->readNext.load();
                return *this;
            }
            iterator operator++(int) noexcept {
                const auto clone = iterator(*this);
                pNode = pNode->readNext.load();
                return clone;
            }

            bool operator==(const iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const iterator& rhs) const noexcept { return pNode != rhs.pNode; }

            bool operator==(const const_iterator& rhs) const noexcept;
            bool operator!=(const const_iterator& rhs) const noexcept;
        };

        class const_iterator {
            friend class fwd_list;

            node_t const* pNode;

            explicit const_iterator(node_t const* pNode) noexcept : pNode(pNode) {}
        public:
            using value_type = const T;
            using difference_type = ptrdiff_t;
            using pointer = T const*;
            using reference = T const&;
            using const_iterator_category = std::forward_iterator_tag;

            const_iterator() noexcept : pNode(nullptr) {}
            ~const_iterator() noexcept = default;

            const_iterator(const_iterator const& clone) noexcept : pNode(clone.pNode) {}
            const_iterator& operator=(const_iterator const& clone) noexcept {
                pNode = clone.pNode;
                return *this;
            }

            T const& operator*() const noexcept { return pNode->val; }
            T const* operator->() const noexcept { return &pNode->val; }

            const_iterator& operator++() noexcept {
                pNode = pNode->readNext.load();
                return *this;
            }
            const_iterator operator++(int) noexcept {
                const auto clone = const_iterator(*this);
                pNode = pNode->readNext.load();
                return clone;
            }

            bool operator==(const const_iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const const_iterator& rhs) const noexcept { return pNode != rhs.pNode; }

            bool operator==(const iterator& rhs) const noexcept { return pNode == rhs.pNode; }
            bool operator!=(const iterator& rhs) const noexcept { return pNode != rhs.pNode; }
        };

        iterator begin() noexcept;
        iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        fwd_list() noexcept;
        ~fwd_list() noexcept;

        template <class...Args>
        T& emplace_front(Args&&...args);

        template <class...Args>
        T& emplace_after(const_iterator it, Args &&...args);
        template <class...Args>
        T& emplace_after(iterator it, Args &&...args);

        T& push_front(T&& old);
        T& push_after(const_iterator it, T &&old);
        T& push_after(iterator it, T &&old);

        T& push_front(T const& clone);
        T& push_after(const_iterator it, T const &clone);
        T& push_after(iterator it, T const &clone);

        fwd_list(fwd_list const&) = delete;
        fwd_list& operator=(fwd_list const&) = delete;
        fwd_list(fwd_list &&) = delete;
        fwd_list& operator=(fwd_list &&) = delete;
    private:
        std::atomic<node_t*> readHead;
        alignas(SC_CACHE_LINE_SIZE) std::atomic<node_t*> writeHead;

        template <class...Args>
        node_t* construct(Args&&...args);
    };

    // ______________
    // Implementation

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::iterator::operator==(const fwd_list<T, alloc_t>::const_iterator &rhs) const noexcept {
        return pNode == rhs.pNode;
    }

    template <class T, template <class> class alloc_t>
    bool fwd_list<T, alloc_t>::iterator::operator!=(const fwd_list<T, alloc_t>::const_iterator &rhs) const noexcept {
        return pNode != rhs.pNode;
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::iterator fwd_list<T, alloc_t>::begin() noexcept {
        return iterator(readHead.load());
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::iterator fwd_list<T, alloc_t>::end() const noexcept {
        return iterator();
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::const_iterator fwd_list<T, alloc_t>::cbegin() const noexcept {
        return const_iterator(readHead.load());
    }

    template <class T, template <class> class alloc_t>
    typename fwd_list<T, alloc_t>::const_iterator fwd_list<T, alloc_t>::cend() const noexcept {
        return const_iterator();
    }

    template <class T, template <class> class alloc_t>
    fwd_list<T, alloc_t>::fwd_list() noexcept :
        readHead(nullptr), writeHead(nullptr)
    {}

    template <class T, template <class> class alloc_t>
    fwd_list<T, alloc_t>::~fwd_list() noexcept {
        auto it = cbegin();
        while (it != cend()) {
            alloc_t<node_t>().deallocate(it++.pNode, 1);
        }
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    T& fwd_list<T, alloc_t>::emplace_front(Args &&... args) {
        auto pNode = construct(std::forward<T>(args)...);

        node_t* pHead;
        do {
            pHead = readHead.load();
            pNode->readNext.store(pHead);
            pNode->writeNext.store(pHead);
        } while (!writeHead.compare_exchange_weak(pHead, pNode));

        readHead.store(pNode);

        return pNode->val;
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    T &fwd_list<T, alloc_t>::emplace_after(fwd_list<T, alloc_t>::const_iterator it, Args &&... args) {
        auto pNode = construct(std::forward<T>(args)...);

        node_t* pCurrent;
        do {
            pCurrent = it.pNode->readNext.load();
            pNode->readNext.store(pCurrent);
            pNode->writeNext.store(pCurrent);
        } while (!it.pNode->writeNext.compare_exchange_weak(pCurrent, pNode));

        it.pNode->readNext.store(pNode);

        return pNode->val;
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    T& fwd_list<T, alloc_t>::emplace_after(iterator it, Args &&... args) {
        return emplace_after(const_iterator(it.pNode), std::forward<T>(args)...);
    }

    template <class T, template <class> class alloc_t>
    T& fwd_list<T, alloc_t>::push_front(T &&old) {
        return emplace_front(std::move(old));
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    T &fwd_list<T, alloc_t>::push_after(const_iterator it, T &&old) {
        return emplace_after(it, std::move(old));
    }

    template <class T, template <class> class alloc_t>
    T& fwd_list<T, alloc_t>::push_after(iterator it, T &&old) {
        return emplace_after(it, std::move(old));
    }

    template <class T, template <class> class alloc_t>
    T& fwd_list<T, alloc_t>::push_front(T const& clone) {
        return emplace_front(clone);
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    T &fwd_list<T, alloc_t>::push_after(const_iterator it, const T &clone) {
        return emplace_after(it, clone);
    }

    template <class T, template <class> class alloc_t>
    T& fwd_list<T, alloc_t>::push_after(iterator it, T const &clone) {
        return emplace_after(it, clone);
    }

    template <class T, template <class> class alloc_t> template<class...Args>
    typename fwd_list<T, alloc_t>::node_t *fwd_list<T, alloc_t>::construct(Args &&... args) {
        node_t* pNode = alloc_t<node_t>().allocate(1);
        new (&pNode->val) T(std::forward<T>(args)...);
        return pNode;
    }

} // End of ::sc
