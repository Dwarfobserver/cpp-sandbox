
#pragma once

#include <utils.hpp>

#include <vector>
#include <tuple>
#include <functional>


namespace sc {

    template<class T, template <class> class Allocator = std::allocator>
    class slot_map {
        class data_t;
    public:
        using iterator = sc::pointer_iterator<slot_map<T, Allocator>, T, sizeof(T) + sizeof(int)>;

        slot_map();

        int size() const noexcept;

        T& operator[](int id) noexcept;
        T const& operator[](int id) const noexcept;

        iterator begin() noexcept;
        iterator end() noexcept;

        void reserve(int capacity);

        void clear() noexcept;

        template <class...Args>
        [[nodiscard]] int emplace(Args &&...args);

        void erase(int id) noexcept;
    private:
        std::vector<data_t, Allocator<T>> objects;
        std::vector<int, Allocator<T>> mapId;
        std::vector<int, Allocator<T>> freeId;
        int _size;

        struct data_t {
            T val;
            int id;

            template <class...Args>
            explicit data_t(int id, Args &&...args) :
                    val(std::forward<Args>(args)...),
                    id(id)
            {}
        };
    };

    // ______________
    // Implementation

    template <class T, template <class> class Allocator>
    slot_map<T, Allocator>::slot_map() :
            objects(),
            mapId(),
            freeId(),
            _size(0)
    {
        static_assert(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>);
    }

    template <class T, template <class> class Allocator>
    inline int slot_map<T, Allocator>::size() const noexcept {
        return _size;
    }

    template <class T, template <class> class Allocator>
    inline T& slot_map<T, Allocator>::operator[](int id) noexcept {
        return objects[mapId[id]].val;
    }

    template <class T, template <class> class Allocator>
    inline T const& slot_map<T, Allocator>::operator[](int id) const noexcept {
        return objects[mapId[id]].val;
    }

    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::iterator slot_map<T, Allocator>::begin() noexcept {
        return iterator(reinterpret_cast<T*>(objects.data()));
    }

    template <class T, template <class> class Allocator>
    inline typename slot_map<T, Allocator>::iterator slot_map<T, Allocator>::end() noexcept {
        return iterator(reinterpret_cast<T*>(objects.data() + _size));
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::reserve(int capacity) {
        auto stdCapacity = static_cast<size_t>(capacity);
        objects.reserve(stdCapacity);
        mapId.reserve(stdCapacity);
        freeId.reserve(stdCapacity);
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::clear() noexcept {
        objects.clear();
        mapId.clear();
        freeId.clear();
        _size = 0;
    }

    template <class T, template <class> class Allocator> template<class...Args>
    [[nodiscard]] int slot_map<T, Allocator>::emplace(Args &&... args) {
        int id;
        if (freeId.empty()) {
            id = _size;
            mapId.push_back(_size);
        } else {
            id = freeId.back();
            freeId.pop_back();
            mapId[id] = _size;
        }
        objects.emplace_back(id, std::forward<Args>(args)...);
        ++_size;
        return id;
    }

    template <class T, template <class> class Allocator>
    void slot_map<T, Allocator>::erase(int id) noexcept {
        int pos = mapId[id];
        data_t& data = objects[pos];

        data = std::move(objects.back());
        mapId[data.id] = pos;
        objects.pop_back();
        --_size;
        freeId.push_back(id);
    }

}
