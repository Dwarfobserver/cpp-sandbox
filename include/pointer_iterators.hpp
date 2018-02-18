
#pragma once

#include <cstddef>
#include <iterator>


namespace sc { // TODO Check std::distance with reverse iterators

    template <class Collection, class T, size_t ALIGN = sizeof(T)>
    class pointer_iterator {
        friend Collection;
        intptr_t ptr;
        explicit pointer_iterator(intptr_t ptr) noexcept : ptr(ptr) {}
        explicit pointer_iterator(void* ptr) noexcept : ptr(reinterpret_cast<intptr_t>(ptr)) {}
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        pointer_iterator() noexcept : ptr(0) {}
        // Default copy, move ctors & dctor

        T& operator*() const noexcept { return *reinterpret_cast<T*>(ptr); }
        T* operator->() const noexcept { return reinterpret_cast<T*>(ptr); }
        T& operator[](int shift) const noexcept { return *reinterpret_cast<T*>(ptr + ALIGN * shift); }

        bool operator==(pointer_iterator it) const noexcept { return ptr == it.ptr; }
        bool operator!=(pointer_iterator it) const noexcept { return ptr != it.ptr; }
        bool operator>=(pointer_iterator it) const noexcept { return ptr >= it.ptr; }
        bool operator<=(pointer_iterator it) const noexcept { return ptr <= it.ptr; }
        bool operator>(pointer_iterator it) const noexcept { return ptr > it.ptr; }
        bool operator<(pointer_iterator it) const noexcept { return ptr < it.ptr; }

        pointer_iterator& operator++() noexcept { ptr += ALIGN; return *this; }
        pointer_iterator& operator--() noexcept { ptr -= ALIGN; return *this; }
        pointer_iterator  operator++(int) noexcept { const auto it = pointer_iterator(ptr); ptr += ALIGN; return it; }
        pointer_iterator  operator--(int) noexcept { const auto it = pointer_iterator(ptr); ptr -= ALIGN; return it; }
        pointer_iterator& operator+=(int shift) noexcept { ptr += ALIGN * shift; return *this; }
        pointer_iterator& operator-=(int shift) noexcept { ptr -= ALIGN * shift; return *this; }
        pointer_iterator  operator+(int shift) const noexcept { return pointer_iterator(ptr + ALIGN * shift); }
        pointer_iterator  operator-(int shift) const noexcept { return pointer_iterator(ptr - ALIGN * shift); }

        difference_type operator-(pointer_iterator it) const noexcept { return (ptr - it.ptr) / ALIGN; }
    };

    template <class Collection, class T, size_t ALIGN = sizeof(T)>
    class const_pointer_iterator {
        friend Collection;
        intptr_t ptr;
        explicit const_pointer_iterator(intptr_t ptr) noexcept : ptr(ptr) {}
        explicit const_pointer_iterator(void* ptr) noexcept : ptr(reinterpret_cast<intptr_t>(ptr)) {}
    public:
        using value_type = T const;
        using difference_type = ptrdiff_t;
        using pointer = T const*;
        using reference = T const&;
        using iterator_category = std::random_access_iterator_tag;

        const_pointer_iterator() noexcept : ptr(0) {}
        // Default copy, move ctors & dctor

        T const& operator*() const noexcept { return *reinterpret_cast<T const*>(ptr); }
        T const* operator->() const noexcept { return reinterpret_cast<T const*>(ptr); }
        T const& operator[](int shift) const noexcept { return *reinterpret_cast<T const*>(ptr + ALIGN * shift); }

        bool operator==(const_pointer_iterator it) const noexcept { return ptr == it.ptr; }
        bool operator!=(const_pointer_iterator it) const noexcept { return ptr != it.ptr; }
        bool operator>=(const_pointer_iterator it) const noexcept { return ptr >= it.ptr; }
        bool operator<=(const_pointer_iterator it) const noexcept { return ptr <= it.ptr; }
        bool operator>(const_pointer_iterator it) const noexcept { return ptr > it.ptr; }
        bool operator<(const_pointer_iterator it) const noexcept { return ptr < it.ptr; }

        const_pointer_iterator& operator++() noexcept { ptr += ALIGN; return *this; }
        const_pointer_iterator& operator--() noexcept { ptr -= ALIGN; return *this; }
        const_pointer_iterator  operator++(int) noexcept { const auto it = const_pointer_iterator(ptr); ptr += ALIGN; return it; }
        const_pointer_iterator  operator--(int) noexcept { const auto it = const_pointer_iterator(ptr); ptr -= ALIGN; return it; }
        const_pointer_iterator& operator+=(int shift) noexcept { ptr += ALIGN * shift; return *this; }
        const_pointer_iterator& operator-=(int shift) noexcept { ptr -= ALIGN * shift; return *this; }
        const_pointer_iterator  operator+(int shift) const noexcept { return const_pointer_iterator(ptr + ALIGN * shift); }
        const_pointer_iterator  operator-(int shift) const noexcept { return const_pointer_iterator(ptr - ALIGN * shift); }

        difference_type operator-(const_pointer_iterator it) const noexcept { return (ptr - it.ptr) / ALIGN; }
    };

    template <class Collection, class T, size_t ALIGN = sizeof(T)>
    class reverse_pointer_iterator {
        friend Collection;
        intptr_t ptr;
        explicit reverse_pointer_iterator(intptr_t ptr) noexcept : ptr(ptr) {}
        explicit reverse_pointer_iterator(void* ptr) noexcept : ptr(reinterpret_cast<intptr_t>(ptr)) {}
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        reverse_pointer_iterator() noexcept : ptr(0) {}
        // Default copy, move ctors & dctor

        T& operator*() const noexcept { return *reinterpret_cast<T*>(ptr); }
        T* operator->() const noexcept { return reinterpret_cast<T*>(ptr); }
        T& operator[](int shift) const noexcept { return *reinterpret_cast<T*>(ptr + ALIGN * shift); }

        bool operator==(reverse_pointer_iterator it) const noexcept { return ptr == it.ptr; }
        bool operator!=(reverse_pointer_iterator it) const noexcept { return ptr != it.ptr; }
        bool operator>=(reverse_pointer_iterator it) const noexcept { return ptr <= it.ptr; }
        bool operator<=(reverse_pointer_iterator it) const noexcept { return ptr >= it.ptr; }
        bool operator>(reverse_pointer_iterator it) const noexcept { return ptr < it.ptr; }
        bool operator<(reverse_pointer_iterator it) const noexcept { return ptr > it.ptr; }

        reverse_pointer_iterator& operator++() noexcept { ptr -= ALIGN; return *this; }
        reverse_pointer_iterator& operator--() noexcept { ptr += ALIGN; return *this; }
        reverse_pointer_iterator  operator++(int) noexcept { const auto it = reverse_pointer_iterator(ptr); ptr -= ALIGN; return it; }
        reverse_pointer_iterator  operator--(int) noexcept { const auto it = reverse_pointer_iterator(ptr); ptr += ALIGN; return it; }
        reverse_pointer_iterator& operator+=(int shift) noexcept { ptr -= ALIGN * shift; return *this; }
        reverse_pointer_iterator& operator-=(int shift) noexcept { ptr += ALIGN * shift; return *this; }
        reverse_pointer_iterator  operator+(int shift) const noexcept { return reverse_pointer_iterator(ptr - ALIGN * shift); }
        reverse_pointer_iterator  operator-(int shift) const noexcept { return reverse_pointer_iterator(ptr + ALIGN * shift); }

        difference_type operator-(reverse_pointer_iterator it) const noexcept { return (it.ptr - ptr) / ALIGN; }
    };

    template <class Collection, class T, size_t ALIGN = sizeof(T)>
    class const_reverse_pointer_iterator {
        friend Collection;
        intptr_t ptr;
        explicit const_reverse_pointer_iterator(intptr_t ptr) noexcept : ptr(ptr) {}
        explicit const_reverse_pointer_iterator(void* ptr) noexcept : ptr(reinterpret_cast<intptr_t>(ptr)) {}
    public:
        using value_type = T const;
        using difference_type = ptrdiff_t;
        using pointer = T const*;
        using reference = T const&;
        using iterator_category = std::random_access_iterator_tag;

        const_reverse_pointer_iterator() noexcept : ptr(0) {}
        // Default copy, move ctors & dctor

        T const& operator*() const noexcept { return *reinterpret_cast<T const*>(ptr); }
        T const* operator->() const noexcept { return reinterpret_cast<T const*>(ptr); }
        T const& operator[](int shift) const noexcept { return *reinterpret_cast<T const*>(ptr + ALIGN * shift); }

        bool operator==(const_reverse_pointer_iterator it) const noexcept { return ptr == it.ptr; }
        bool operator!=(const_reverse_pointer_iterator it) const noexcept { return ptr != it.ptr; }
        bool operator>=(const_reverse_pointer_iterator it) const noexcept { return ptr <= it.ptr; }
        bool operator<=(const_reverse_pointer_iterator it) const noexcept { return ptr >= it.ptr; }
        bool operator>(const_reverse_pointer_iterator it) const noexcept { return ptr < it.ptr; }
        bool operator<(const_reverse_pointer_iterator it) const noexcept { return ptr > it.ptr; }

        const_reverse_pointer_iterator& operator++() noexcept { ptr -= ALIGN; return *this; }
        const_reverse_pointer_iterator& operator--() noexcept { ptr += ALIGN; return *this; }
        const_reverse_pointer_iterator  operator++(int) noexcept { const auto it = const_reverse_pointer_iterator(ptr); ptr -= ALIGN; return it; }
        const_reverse_pointer_iterator  operator--(int) noexcept { const auto it = const_reverse_pointer_iterator(ptr); ptr += ALIGN; return it; }
        const_reverse_pointer_iterator& operator+=(int shift) noexcept { ptr -= ALIGN * shift; return *this; }
        const_reverse_pointer_iterator& operator-=(int shift) noexcept { ptr += ALIGN * shift; return *this; }
        const_reverse_pointer_iterator  operator+(int shift) const noexcept { return const_reverse_pointer_iterator(ptr - ALIGN * shift); }
        const_reverse_pointer_iterator  operator-(int shift) const noexcept { return const_reverse_pointer_iterator(ptr + ALIGN * shift); }

        difference_type operator-(const_reverse_pointer_iterator it) const noexcept { return (it.ptr - ptr) / ALIGN; }
    };

}
