
#pragma once

#include <cstring>
#include "pointer_iterators.hpp"


namespace sc {

    template <class T, class Allocator = std::allocator<T>>
    class pod_vector {
    public:
        static float growing_factor();

        using value_type = T;
        using allocator_type = Allocator;
        using size_type = int;

        using iterator = pointer_iterator<pod_vector<T>, T>;
        using const_iterator = const_pointer_iterator<pod_vector<T>, T>;
        using reverse_iterator = reverse_pointer_iterator<pod_vector<T>, T>;
        using const_reverse_iterator = const_reverse_pointer_iterator<pod_vector<T>, T>;

        explicit pod_vector(Allocator const& allocator = Allocator()) noexcept;
        ~pod_vector() noexcept;
        pod_vector(pod_vector const& clone);
        pod_vector(pod_vector&& clone) noexcept;
        pod_vector& operator=(pod_vector const& clone);
        pod_vector& operator=(pod_vector&& clone) noexcept;

        explicit pod_vector(int size, Allocator const& allocator = Allocator());

        void reserve(int capacity);
        void resize(int size);
        void shrink_to_fit();
        void clear()         { size_ = 0; }
        int size() const     { return size_; }
        int capacity() const { return capacity_; }
        bool empty() const   { return size_ == 0; }

        T* data()                        { return data_; }
        T const* data() const            { return data_; }
        T& operator[](int i)             { return data_[i]; }
        T const& operator[](int i) const { return data_[i]; }
        T& front()                       { return data_[0]; }
        T const& front() const           { return data_[0]; }
        T& back()                        { return data_[size_ - 1]; }
        T const& back() const            { return data_[size_ - 1]; }

        template <class...Args>
        T& emplace_back(Args&&...args);
        void pop_back();


        iterator begin()                       { return iterator(data_); }
        iterator end()                         { return iterator(data_ + size_); }
        const_iterator cbegin() const          { return const_iterator(data_); }
        const_iterator cend() const            { return const_iterator(data_ + size_); }
        reverse_iterator rbegin()              { return reverse_iterator(data_ + size_ - 1); }
        reverse_iterator rend()                { return reverse_iterator(data_ - 1); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(data_ + size_ - 1); }
        const_reverse_iterator crend() const   { return const_reverse_iterator(data_ - 1); }
    private:
        void reallocate(int capacity);

        int size_;
        int capacity_;
        T* data_;
        Allocator allocator_;
    };

    template<class T, class Allocator>
    float pod_vector<T, Allocator>::growing_factor() {
        return 2.f;
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator>::pod_vector(Allocator const& allocator) noexcept :
            size_{0},
            capacity_{0},
            data_{nullptr},
            allocator_{allocator}
    {}

    template<class T, class Allocator>
    pod_vector<T, Allocator>::~pod_vector() noexcept {
        std::allocator_traits<Allocator>::deallocate(allocator_, data_, capacity_);
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator>::pod_vector(pod_vector const &clone) :
            size_{clone.size_},
            capacity_{clone.size_},
            data_{nullptr},
            allocator_{clone.allocator_}
    {
        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity_);
        std::memcpy(data_, clone.data_, static_cast<size_t>(size_));
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator>::pod_vector(pod_vector &&clone) noexcept :
            size_{clone.size_},
            capacity_{clone.capacity_},
            data_{clone.data_},
            allocator_{std::move(clone.allocator_)}
    {
        clone.data_ = nullptr;
        clone.size_ = 0;
        clone.capacity_ = 0;
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator> &pod_vector<T, Allocator>::operator=(pod_vector const &clone) {
        size_ = clone.size_;
        if (capacity_ < size_) {
            std::allocator_traits<Allocator>::deallocate(allocator_, data_, capacity_);
            data_ = std::allocator_traits<Allocator>::allocate(allocator_, size_);
            capacity_ = size_;
        }
        std::memcpy(data_, clone.data_, static_cast<size_t>(size_));
        return *this;
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator> &pod_vector<T, Allocator>::operator=(pod_vector &&clone) noexcept {
        std::allocator_traits<Allocator>::deallocate(allocator_, data_, capacity_);
        size_ = clone.size_;
        capacity_ = clone.capacity_;
        data_ = clone.data_;
        allocator_ = std::move(clone.allocator_);

        clone.data_ = nullptr;
        clone.size_ = 0;
        clone.capacity_ = 0;

        return *this;
    }

    template<class T, class Allocator>
    pod_vector<T, Allocator>::pod_vector(int size, const Allocator &allocator) :
            size_{size},
            capacity_{size},
            data_{nullptr},
            allocator_{allocator}
    {
        data_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity_);
    }

    template<class T, class Allocator>
    void pod_vector<T, Allocator>::reserve(int capacity) {
        if (capacity <= capacity_) return;
        reallocate(capacity);
    }

    template<class T, class Allocator>
    void pod_vector<T, Allocator>::resize(int size) {
        if (capacity_ < size) {
            reallocate(size);
        }
        size_ = size;
    }

    template<class T, class Allocator>
    void pod_vector<T, Allocator>::shrink_to_fit() {
        if (size_ == capacity_) return;

        reallocate(size_);
    }

    template<class T, class Allocator>
    template<class... Args>
    T &pod_vector<T, Allocator>::emplace_back(Args &&... args) {
        if (size_ == capacity_) {
            reallocate(static_cast<int>(size_ * growing_factor()));
        }
        ++size_;
        new (&back()) T(std::forward<Args>(args)...);
        return back();
    }

    template<class T, class Allocator>
    void pod_vector<T, Allocator>::pop_back() {
        --size_;
    }

    template<class T, class Allocator>
    void pod_vector<T, Allocator>::reallocate(int capacity) {
        T* const data = std::allocator_traits<Allocator>::allocate(allocator_, capacity);
        std::memcpy(data, data_, static_cast<size_t>(size_));
        std::allocator_traits<Allocator>::deallocate(allocator_, data_, capacity_);
        capacity_ = capacity;
        data_ = data;
    }


}
