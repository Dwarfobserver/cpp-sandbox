
#pragma once

#include <bits/allocator.h>
#include <atomic>

/*
namespace sc {

    template <class T, class alloc_t = std::allocator<T>>
    class fwd_list {
    public:
        fwd_list();

        template <class...Args>
        T& emplace_front(Args&&...args);

    private:
        struct node_t {
            std::atomic<T*> val;
            std::atomic<node_t*> next;
        };
        using alloc_node_t = alloc_t::rebind<node_t>;

        std::atomic<node_t*> readHead;
        std::atomic<node_t*> writeHead;
    };

    template <class T, template <class> class Allocator>
    fwd_list<T, Allocator>::fwd_list() :
        readHead(nullptr), writeHead(nullptr)
    {
    }

    template <class T, class alloc_t> template<class...Args>
    T& fwd_list<T, alloc_t>::emplace_front(Args &&... args) {
        node_t* pNode = alloc_node_t().allocate(1);
        new (&pNode->val) T(std::forward<T>(args)...);

        auto pFirst = writeHead.exchange(pNode);

        pNode->next.store(pFirst); // Must be set at head exchange

        return pNode->val;
    }

} // End of ::sc
*/