
#pragma once


#include <memory>

namespace sc {

    template <class T>
    class stack_allocator;

    class stack_resource {
    public:
        explicit stack_resource(int capacity) :
                data_(std::make_unique<char[]>(static_cast<size_t>(capacity))),
                size_(0),
                capacity_(capacity)
        {}
        stack_resource(stack_resource&&) = delete;

        void* push(int nb) {
            if (nb > capacity_ - size_) throw std::runtime_error
                { "stack_resource could not provide enough memory at push() call." };
            void* ptr = data_.get() + size_;
            size_ += nb;
            return ptr;
        }

        void pop(int nb) {
            if (nb > size_) throw std::runtime_error
                { "stack_resource could not free enough memory at pop() call." };
            size_ -= nb;
        }

        template <class T>
        stack_allocator<T> get_allocator() {
            return stack_allocator<T>{ *this };
        }

        int capacity() const { return capacity_; }
        int size() const     { return size_; }
    private:
        std::unique_ptr<char[]> data_;
        int size_;
        const int capacity_;
    };

    class stack_guard {
    public:
        explicit stack_guard(stack_resource& stack) :
                stack_(stack),
                index_(stack.size())
        {}
        stack_guard(stack_guard&&) = delete;

        ~stack_guard() noexcept {
            stack_.pop(stack_.size() - index_);
        }

        template <class T>
        stack_allocator<T> get_allocator() { // TODO Track allocators in debug
            return stack_.get_allocator<T>();
        }
    private:
        stack_resource& stack_;
        int index_;
    };

    template <class T>
    class stack_allocator {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_copy_assignment = std::true_type;
        using is_always_equal = std::false_type;

        template <class U>
        struct rebind { using other = stack_allocator<U>; };

        explicit stack_allocator(stack_resource& resource) : resource_(resource) {}
        stack_allocator(stack_allocator&& allocator) noexcept = default;
        stack_allocator(stack_allocator const& allocator) = default;
        template <class U>
        explicit stack_allocator(stack_allocator<U>& allocator) : resource_(allocator.resource_) {}

        T* allocate(size_t nb) { return static_cast<int*>(resource_.push(sizeof(T) * nb)); }
        void deallocate(T *pChunk, size_t nb) {}
    private:
        stack_resource& resource_;
    };

    template <class T, class U>
    bool operator==(stack_allocator<T> const& lhs, stack_allocator<U> const& rhs) {
        return &lhs.resource_ == &rhs.resource_;
    };
    template <class T, class U>
    bool operator!=(stack_allocator<T> const& lhs, stack_allocator<U> const& rhs) {
        return &lhs.resource_ != &rhs.resource_;
    };
}
