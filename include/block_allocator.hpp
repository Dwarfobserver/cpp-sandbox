
#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <forward_list>
#include "pod_vector.hpp"


namespace sc {

    template <bool DYNAMIC, class T>
    class block_allocator_resource;

    // Static resource

    template <class T>
    class block_allocator_resource<false, T> {
    public:
        explicit block_allocator_resource(int size) :
            size_(0),
            blocks_(size)
        {
            for (int i = 0; i < size - 1; ++i) {
                blocks_[i].pNext = blocks_.data() + i + 1;
            }
            blocks_.back().pNext = nullptr;
            pNext_ = blocks_.data();
        }
        block_allocator_resource(block_allocator_resource&&) = delete;
        block_allocator_resource& operator=(block_allocator_resource&&) = delete;

        T* allocate() {
            assert(pNext_ != nullptr);
            auto ptr = reinterpret_cast<T*>(pNext_);
            pNext_ = pNext_->pNext;
            ++size_;
            return ptr;
        }
        void deallocate(T* ptr) {
            auto nodePtr = reinterpret_cast<node_t*>(ptr);
            nodePtr->pNext = pNext_;
            pNext_ = nodePtr;
            --size_;
        }

        int size() const { return size_; }
        int capacity() const { return blocks_.size(); }
    private:
        struct alignas(alignof(T)) node_t {
            union {
                std::aligned_storage_t<sizeof(T), alignof(T)> storage;
                node_t* pNext;
            };
        };

        int size_;
        node_t* pNext_;
        sc::pod_vector<node_t> blocks_;
    };

    // Dynamic resource

    template <class T>
    class block_allocator_resource<true, T> {
    public:
        explicit block_allocator_resource(int size) :
                size_(0),
                blocksCount_(0),
                blocksSize_(size)
        {
            make_blocks();
        }
        block_allocator_resource(block_allocator_resource&&) = delete;
        block_allocator_resource& operator=(block_allocator_resource&&) = delete;

        T* allocate() {
            if (pNext_ == nullptr) {
                make_blocks();
            }
            auto ptr = reinterpret_cast<T*>(pNext_);
            pNext_ = pNext_->pNext;
            ++size_;
            return ptr;
        }
        void deallocate(T* ptr) {
            auto nodePtr = reinterpret_cast<node_t*>(ptr);
            nodePtr->pNext = pNext_;
            pNext_ = nodePtr;
            --size_;
        }

        int size() const { return size_; }
        int capacity() const { return blocksCount_ * blocksSize_; }
    private:
        void make_blocks() {
            ++blocksCount_;
            auto& vec = blocks_.emplace_front(blocksSize_);
            for (int i = 0; i < blocksSize_ - 1; ++i) {
                vec[i].pNext = vec.data() + i + 1;
            }
            vec.back().pNext = nullptr;
            pNext_ = vec.data();
        }

        struct alignas(alignof(T)) node_t {
            union {
                std::aligned_storage_t<sizeof(T), alignof(T)> storage;
                node_t* pNext;
            };
        };

        int size_;
        node_t* pNext_;
        int blocksCount_;
        int blocksSize_;
        std::forward_list<sc::pod_vector<node_t>> blocks_;
    };

    // Allocator

    template <bool DYNAMIC, class T>
    class block_allocator {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_copy_assignment = std::true_type;
        using is_always_equal = std::false_type;

        explicit block_allocator(block_allocator_resource<DYNAMIC, T>& resource) : resource_(resource) {}

        // Must not be rebind

        T* allocate(size_t nb) {
            assert(nb == 1);
            return resource_.allocate();
        }
        void deallocate(T* ptr, size_t nb) {
            assert(nb == 1);
            resource_.deallocate(ptr);
        }
    private:
        block_allocator_resource<DYNAMIC, T>& resource_;
    };

    template <bool DYNAMIC, class T, class U>
    bool operator==(block_allocator<DYNAMIC, T> const& lhs, block_allocator<DYNAMIC, U> const& rhs) {
        return &lhs.resource_ == &rhs.resource_;
    };
    template <bool DYNAMIC, class T, class U>
    bool operator!=(block_allocator<DYNAMIC, T> const& lhs, block_allocator<DYNAMIC, U> const& rhs) {
        return &lhs.resource_ != &rhs.resource_;
    };

}
