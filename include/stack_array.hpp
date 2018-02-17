
#pragma once

#include <malloc.h>
#include <pointer_iterators.hpp>

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define SC_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define SC_ALWAYS_INLINE __forceinline
#else
#error Compiler not supported
#endif

namespace sc {

    template <class T>
    class stack_array {
    public:
        struct uninitialized_tag {};

        using value_type = T;
        using size_type = int;

        using iterator = pointer_iterator<stack_array, T>;
        using const_iterator = const_pointer_iterator<stack_array, T>;
        using reverse_iterator = reverse_pointer_iterator<stack_array, T>;
        using const_reverse_iterator = const_reverse_pointer_iterator<stack_array, T>;

        SC_ALWAYS_INLINE stack_array(int size, uninitialized_tag);
        SC_ALWAYS_INLINE explicit stack_array(int size);
        SC_ALWAYS_INLINE stack_array(int size, T const& val);
        SC_ALWAYS_INLINE stack_array(std::initializer_list<T> vals);
        template <class It>
        SC_ALWAYS_INLINE stack_array(It begin, It end);

        ~stack_array() noexcept(std::is_nothrow_destructible_v<T>);

        stack_array(stack_array&&)                 = delete;
        stack_array(stack_array const&)            = delete;
        stack_array& operator=(stack_array&&)      = delete;
        stack_array& operator=(stack_array const&) = delete;

        int size() const { return size_; }

        iterator begin()                       { return iterator(data_); }
        iterator end()                         { return iterator(data_ + size_); }
        const_iterator cbegin() const          { return const_iterator(data_); }
        const_iterator cend() const            { return const_iterator(data_ + size_); }
        reverse_iterator rbegin()              { return reverse_iterator(data_ + size_ - 1); }
        reverse_iterator rend()                { return reverse_iterator(data_ - 1); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(data_ + size_ - 1); }
        const_reverse_iterator crend() const   { return const_reverse_iterator(data_ - 1); }

        T* data()                        { return data_; }
        T const* data() const            { return data_; }
        T& operator[](int i)             { return data_[i]; }
        T const& operator[](int i) const { return data_[i]; }
        T& at(int i)                     { check_bounds(i); return data_[i]; }
        T const& at(int i) const         { check_bounds(i); return data_[i]; }
        T& front()                       { return data_[0]; }
        T const& front() const           { return data_[0]; }
        T& back()                        { return data_[size_ - 1]; }
        T const& back() const            { return data_[size_ - 1]; }
    private:
        void check_bounds(int pos) const;

        int size_;
        T* data_;
    };

#undef SC_ALWAYS_INLINE

    // ______________
    // Implementation

    template<class T>
    stack_array<T>::stack_array(int size, uninitialized_tag) :
            size_(size), data_(static_cast<T*>(alloca(size * sizeof(T)))) {}

    template<class T>
    stack_array<T>::stack_array(int size) :
            stack_array(size, uninitialized_tag{})
    {
        for (int i = 0; i < size_; ++i) new (data_ + i) T();
    }

    template<class T>
    stack_array<T>::stack_array(int size, const T &val) :
            stack_array(size, uninitialized_tag{})
    {
        for (int i = 0; i < size_; ++i) new (data_ + i) T(val);
    }

    template<class T>
    stack_array<T>::stack_array(std::initializer_list<T> vals) :
            stack_array(vals.size(), uninitialized_tag{})
    {
        int i = 0;
        for (T const& val : vals) new (data_ + i++) T(val);
    }

    template<class T>
    template<class It>
    stack_array<T>::stack_array(It begin, It end) :
            stack_array(std::distance(begin, end), uninitialized_tag{})
    {
        int i = 0;
        while (begin != end) new (data_ + i++) T(*(begin++));
    }

    template<class T>
    stack_array<T>::~stack_array() noexcept(std::is_nothrow_destructible_v<T>) {
        for (int i = 0; i < size_; ++i) (data_ + i)->~T();
    }

    template<class T>
    void stack_array<T>::check_bounds(int pos) const {
        if (pos < 0 || pos >= size_) throw std::out_of_range
            {"Invalid index value at a call of std::stack_array::at(i) : "
                 "i = " + std::to_string(pos) + ", "
                 "size = " + std::to_string(size_) };
    }

}
