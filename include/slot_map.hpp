
#pragma once

#include "pointer_iterators.hpp"

#include <vector>
#include <tuple>
#include <functional>
#include <cassert>


namespace sc {

    template<class T, template <class> class Allocator = std::allocator>
    class slot_map { // To falsify keys : store gen in indices
        class data_t;
    public:
        struct key {
            friend class slot_map;
            key() = default;
            key(key const&) = default;
            key& operator=(key const&) = default;
        private:
            key(int pos, unsigned char gen) noexcept;
            int pos() const noexcept;
            unsigned char gen() const noexcept;
            key nextGen() const noexcept;

            static constexpr int GEN_MASK = (1 << 24) - 1;
            int value;
        };

        using iterator = sc::pointer_iterator<slot_map<T, Allocator>, T, sizeof(data_t)>;
        using const_iterator = sc::const_pointer_iterator<slot_map<T, Allocator>, T, sizeof(data_t)>;
        using reverse_iterator = sc::reverse_pointer_iterator<slot_map<T, Allocator>, T, sizeof(data_t)>;
        using const_reverse_iterator = sc::const_reverse_pointer_iterator<slot_map<T, Allocator>, T, sizeof(data_t)>;

        slot_map() noexcept;

        int size() const noexcept;

        T& operator[](key k) noexcept;
        T const& operator[](key k) const noexcept;

        T* try_get(key k) noexcept;
        T const* try_get(key k) const noexcept;

        iterator begin() noexcept;
        iterator end() noexcept;
        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;
        reverse_iterator rbegin() noexcept;
        reverse_iterator rend() noexcept;
        const_reverse_iterator crbegin() const noexcept;
        const_reverse_iterator crend() const noexcept;

        void reserve(int capacity);

        void clear() noexcept;

        template <class...Args>
        [[nodiscard]] key emplace(Args &&...args);

        void erase(key k) noexcept;

        key get_key(T &val) const noexcept;
    private:
        std::vector<data_t, Allocator<T>> objects;
        std::vector<key, Allocator<T>> indices;
        std::vector<key, Allocator<T>> freeKeys;
        int size_;

        struct alignas(T) data_t {
            T val;
            key k;

            template <class...Args>
            explicit data_t(key k, Args &&...args) :
                    val(std::forward<Args>(args)...),
                    k(k)
            {}
        };
    };

    // ______________
    // Implementation

    // Key

    template <class T, template <class> class Allocator>
    slot_map<T, Allocator>::key::key(int pos, unsigned char gen) noexcept :
            value((pos & GEN_MASK) + (gen << 24))
    {}

    template <class T, template <class> class Allocator>
    int slot_map<T, Allocator>::key::pos() const noexcept {
        return value & GEN_MASK;
    }

    template <class T, template <class> class Allocator>
    unsigned char slot_map<T, Allocator>::key::gen() const noexcept {
        return static_cast<unsigned char>((value & ~GEN_MASK) >> 24);
    }

    template <class T, template <class> class Allocator>
    typename slot_map<T, Allocator>::key slot_map<T, Allocator>::key::nextGen() const noexcept {
        return {pos(), static_cast<unsigned char>(gen() + 1)};
    }

    // Slot map

    template <class T, template <class> class Allocator>
    slot_map<T, Allocator>::slot_map() noexcept :
            objects(),
            indices(),
            freeKeys(),
            size_(0)
    {
        static_assert(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>);
    }

    template <class T, template <class> class Allocator>
    inline int slot_map<T, Allocator>::size() const noexcept {
        return size_;
    }

    template <class T, template <class> class Allocator>
    inline T& slot_map<T, Allocator>::operator[](key k) noexcept {
        key k2 = indices[k.pos()];
        assert(k2.gen() == k.gen() && "The key is not valid anymore (the object has been deleted");
        return objects[k2.pos()].val;
    }

    template <class T, template <class> class Allocator>
    inline T const& slot_map<T, Allocator>::operator[](key k) const noexcept {
        key k2 = indices[k.pos()];
        assert(k2.gen() == k.gen() && "The key is not valid anymore (the object has been deleted");
        return objects[k2.pos()].val;
    }

    template <class T, template <class> class Allocator>
    T *slot_map<T, Allocator>::try_get(key k) noexcept {
        key k2 = indices[k.pos()];
        return k2.gen() == k.gen() ? &objects[k2.pos()].val : nullptr;
    }

    template <class T, template <class> class Allocator>
    T const *slot_map<T, Allocator>::try_get(key k) const noexcept {
        key k2 = indices[k.pos()];
        return k2.gen() == k.gen() ? &objects[k2.pos()].val : nullptr;
    }

    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::iterator slot_map<T, Allocator>::begin() noexcept {
        return iterator(objects.data());
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::iterator slot_map<T, Allocator>::end() noexcept {
        return iterator(objects.data() + size_);
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::const_iterator slot_map<T, Allocator>::cbegin() const noexcept {
        return const_iterator(objects.data());
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::const_iterator slot_map<T, Allocator>::cend() const noexcept {
        return const_iterator(objects.data() + size_);
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::reverse_iterator slot_map<T, Allocator>::rbegin() noexcept {
        return reverse_iterator(objects.back());
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::reverse_iterator slot_map<T, Allocator>::rend() noexcept {
        return reverse_iterator(objects.data() - 1);
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::const_reverse_iterator slot_map<T, Allocator>::crbegin() const noexcept {
        return const_reverse_iterator(objects.back());
    }
    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::const_reverse_iterator slot_map<T, Allocator>::crend() const noexcept {
        return const_reverse_iterator(objects.data() - 1);
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::reserve(int capacity) {
        auto stdCapacity = static_cast<size_t>(capacity);
        objects.reserve(stdCapacity);
        indices.reserve(stdCapacity);
        freeKeys.reserve(stdCapacity);
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::clear() noexcept {
        objects.clear();
        indices.clear();
        freeKeys.clear();
        size_ = 0;
    }

    template <class T, template <class> class Allocator> template<class...Args>
    [[nodiscard]] typename slot_map<T, Allocator>::key slot_map<T, Allocator>::emplace(Args &&... args) {
        key k;
        if (freeKeys.empty()) {
            k = key(size_, 0);
            indices.push_back(k);
        } else {
            k = freeKeys.back();
            freeKeys.pop_back();
            indices[k.pos()] = {size_, k.gen()};
        }
        objects.emplace_back(k, std::forward<Args>(args)...);
        ++size_;
        return k;
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::erase(key k) noexcept {
        key k2 = indices[k.pos()];
        assert(k2.gen() == k.gen() && "The key is not valid anymore (the object has been deleted");
        data_t& data = objects[k2.pos()];
        data_t& back = objects.back();

        if (&data != &back) {
            data = std::move(objects.back());
            indices[data.k.pos()] = k2;
            indices[k.pos()] = k2.nextGen();
        }

        objects.pop_back();
        --size_;
        freeKeys.push_back(k.nextGen());
    }

    template <class T, template <class> class Allocator>
    typename slot_map<T, Allocator>::key slot_map<T, Allocator>::get_key(T &val) const noexcept {
        return reinterpret_cast<data_t*>(&val)->k;
    }

}
