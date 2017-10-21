
#pragma once

#include <vector>
#include <tuple>
#include <functional>


namespace slot_array { // TODO Add objects destructors
    using id_type = int;
    using size_type = int;

    namespace traits {
        // TODO
    }

    template<class T>
    class type {
        using pair = std::pair<T, id_type>;

        struct iterator_type {
        private:
            pair* ptr;
        public:
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::random_access_iterator_tag;


            iterator_type() = default;
            explicit iterator_type(pair* ptr) : ptr{ptr} {}
            iterator_type(iterator_type const& rhs) = default;

            iterator_type& operator=(iterator_type const& rhs) { ptr = rhs.ptr; return *this; }

            bool operator==(const iterator_type& rhs) { return ptr == rhs.ptr; }
            bool operator!=(const iterator_type& rhs) { return ptr != rhs.ptr; }

            T& operator*() { return ptr->first; }
            T* operator->() { return &ptr->first; }

            iterator_type& operator++() { ++ptr; return *this; }
            iterator_type& operator--() { --ptr; return *this; }

            iterator_type& operator++(int) { ptr++; return *this; }
            iterator_type& operator--(int) { ptr--; return *this; }

            iterator_type operator+(int shift) { return iterator_type{ ptr + shift }; }
            iterator_type operator-(int shift) { return iterator_type{ ptr - shift }; }

            void operator+=(int shift) { ptr += shift; }
            void operator-=(int shift) { ptr -= shift; }
        };
    public:
        using iterator = iterator_type;
        using const_iterator = iterator_type;
    private:
        std::vector<pair> objects;
        std::vector<size_type> mapId;
        std::vector<id_type> freeId;
        size_type objectsSize;
    public:
        explicit type(size_type initialSize = 10) :
                objectsSize { 0 }
        {
            auto stdSize = static_cast<unsigned long long int>(initialSize);
            objects.reserve(stdSize);
            mapId.reserve(stdSize);
            freeId.reserve(stdSize);
        }

        inline size_type size() {
            return objectsSize;
        }

        inline T& operator[](id_type id) {
            return objects[mapId[id]].first;
        }

        inline T const& operator[](id_type id) const {
            return objects[mapId[id]].first;
        }

        inline iterator begin() { return iterator_type{&objects[0]}; }
        inline iterator end() { return iterator_type{&objects[objectsSize]}; }

        [[nodiscard]]
        id_type insert(T const& value) {
            id_type id;
            if (freeId.empty()) {
                id = objectsSize;
                mapId.push_back(objectsSize);
                objects.emplace_back(value, id);
            } else {
                id = freeId.back();
                freeId.pop_back();
                mapId[id] = objectsSize;
                objects[objectsSize].first = value;
                objects[objectsSize].second = id;
            }
            ++objectsSize;
            return id;
        }

        void erase(id_type id) {
            auto index = mapId[id];
            --objectsSize;
            mapId[objects[objectsSize].second] = index;
            objects[index].first = objects[objectsSize].first;
            freeId.push_back(id);
        }

        void clear() {
            for (int id : mapId) freeId.push_back(id);
            mapId.clear();
            objects.clear();
            objectsSize = 0;
        }
    };

}


#endif //CLION_SANDBOX_SLOTMAP_HPP
