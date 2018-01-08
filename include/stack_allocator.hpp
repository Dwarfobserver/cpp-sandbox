
#pragma once

#include <cstddef>
#include <cassert>
#include <malloc.h>
#include <type_traits>
#include <pod_vector.hpp>
#include <forward_list>
#include <iostream>

namespace sc {

    template <bool DYNAMIC>
    class stack_allocator_resource;

    namespace detail {
        template <class T>
        T* get_next_align(T* val) {
            return reinterpret_cast<T*>(static_cast<uintptr_t>(
                ((reinterpret_cast<uintptr_t>(val) + alignof(T) - 1) / alignof(T)) * alignof(T)
            ));
        }
    }

    // Static resource

    template <>
    class stack_allocator_resource<false> {
    public:
        explicit stack_allocator_resource(int size) : stack_(size) {
            ptr_ = stack_.data();
        }
        stack_allocator_resource(stack_allocator_resource&&) = delete;
        stack_allocator_resource& operator=(stack_allocator_resource&&) = delete;

        template <class T>
        T* allocate(size_t nb) {
            auto ptr = detail::get_next_align(reinterpret_cast<T*>(ptr_));
            ptr_ = reinterpret_cast<std::byte*>(ptr) + nb * sizeof(T);
            assert(ptr_ <= stack_.data() + stack_.size());
            return ptr;
        }

        int size() const { return static_cast<int>(ptr_ - stack_.data()); }
        int capacity() const { return stack_.size(); }
    private:
        std::byte* ptr_;
        sc::pod_vector<std::byte> stack_;
    };

    // Dynamic resource

    template <>
    class stack_allocator_resource<true> {
    public:
        explicit stack_allocator_resource(int size) :
                chunkSize_(size),
                chunkCount_(0)
        {
            stack_.emplace_front(chunkSize_);
            ++chunkCount_;
            ptr_ = stack_.front().data();
        }
        stack_allocator_resource(stack_allocator_resource&&) = delete;
        stack_allocator_resource& operator=(stack_allocator_resource&&) = delete;

        template <class T>
        T* allocate(size_t nb) {
            auto ptr = detail::get_next_align(reinterpret_cast<T*>(ptr_));
            ptr_ = reinterpret_cast<std::byte*>(ptr) + nb * sizeof(T);
            if (ptr_ > stack_.front().data() + chunkSize_) {
                stack_.emplace_front(chunkSize_);
                ++chunkCount_;
                ptr = detail::get_next_align(reinterpret_cast<T*>(stack_.front().data()));
                ptr_ = reinterpret_cast<std::byte*>(ptr) + nb * sizeof(T);
            }
            return ptr;
        }

        int size() const { return chunkSize_ * (chunkCount_ - 1) + static_cast<int>(ptr_ - stack_.front().data()); }
        int capacity() const { return chunkSize_ * chunkCount_; }
    private:
        int chunkSize_;
        int chunkCount_;
        std::byte* ptr_;
        std::forward_list<sc::pod_vector<std::byte>> stack_;
    };

    // Allocator

    template <bool DYNAMIC, class T>
    class stack_allocator {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_copy_assignment = std::true_type;
        using is_always_equal = std::false_type;

        explicit stack_allocator(stack_allocator_resource<DYNAMIC>& resource) : resource_(resource) {}

        template <class U>
        struct rebind { using other = stack_allocator<DYNAMIC, U>; };

        T* allocate(size_t nb) {
            return resource_.template allocate<T>(nb);
        }
        void deallocate(T *pChunk, size_t nb) {}

    private:
        stack_allocator_resource<DYNAMIC>& resource_;
    };

    template <bool DYNAMIC, class T, class U>
    bool operator==(stack_allocator<DYNAMIC, T> const& lhs, stack_allocator<DYNAMIC, U> const& rhs) {
        return &lhs.resource_ == &rhs.resource_;
    };
    template <bool DYNAMIC, class T, class U>
    bool operator!=(stack_allocator<DYNAMIC, T> const& lhs, stack_allocator<DYNAMIC, U> const& rhs) {
        return &lhs.resource_ != &rhs.resource_;
    };

}
