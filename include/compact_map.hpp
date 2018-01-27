
#pragma once

#include <vector>
#include <functional>
#include <pointer_iterators.hpp>


namespace sc {

    template <
            class Key,
            class T,
            class Comparator = std::less<Key>,
            class Allocator = std::allocator<std::pair<const Key, T>>
    >
    class compact_map { // TODO const Key, then const_cast to change them
        using mut_value_type = std::pair<Key, T>;
    public:
        using key_type    = Key;
        using mapped_type = T;
        using value_type  = std::pair<const Key, T>;

        using iterator               = typename std::vector<value_type, Allocator>::iterator;
        using const_iterator         = typename std::vector<value_type, Allocator>::const_iterator;
        using reverse_iterator       = typename std::vector<value_type, Allocator>::reverse_iterator;
        using const_reverse_iterator = typename std::vector<value_type, Allocator>::const_reverse_iterator;

        explicit compact_map(Allocator const& allocator = Allocator());
        explicit compact_map(Comparator const& comparator, Allocator const& allocator = Allocator());

        iterator begin() noexcept                       { return vector_.begin(); }
        iterator end()   noexcept                       { return vector_.end(); }
        const_iterator cbegin() const noexcept          { return vector_.cbegin(); }
        const_iterator cend()   const noexcept          { return vector_.cend(); }
        reverse_iterator rbegin() noexcept              { return vector_.rbegin(); }
        reverse_iterator rend()   noexcept              { return vector_.rend(); }
        const_reverse_iterator crbegin() const noexcept { return vector_.crbegin(); }
        const_reverse_iterator crend()   const noexcept { return vector_.crend(); }

        bool empty() const noexcept   { return vector_.empty(); }
        int size() const noexcept     { return static_cast<int>(vector_.size()); }
        int capacity() const noexcept { return static_cast<int>(vector_.capacity()); }

        void resize(int size)                        { vector_.resize(static_cast<uint64_t>(size)); }
        void resize(int size, value_type const& val) { vector_.resize(static_cast<uint64_t>(size), val); }
        void reserve(int size)                       { vector_.reserve(static_cast<uint64_t>(size)); }
        void shrink_to_fit()                         { vector_.shrink_to_fit(); }
        void clear() noexcept                        { vector_.clear(); }

        value_type* data() noexcept              { return vector_.data(); }
        value_type const* data() const noexcept  { return vector_.data(); }
        value_type& front() noexcept             { return vector_.front(); }
        value_type const& front() const noexcept { return vector_.front(); }
        value_type& back() noexcept              { return vector_.back(); }
        value_type const& back() const noexcept  { return vector_.back(); }

        Allocator get_allocator() const { return vector_.get_allocator(); }

        T& operator[](Key const& key);
        T& at(Key const& key);
        T const& at(Key const& key) const;
        iterator find(Key const& key);
        const_iterator find(Key const& key) const;

        std::pair<iterator, bool> insert(value_type const& val);
        iterator erase(iterator it);
    private:
        std::pair<int, bool> dichotomy_search(Key const& key, int min, int max) const;

        Comparator comparator_;
        std::vector<value_type, Allocator> vector_;
    };

    template<class Key, class T, class Comparator, class Allocator>
    compact_map<Key, T, Comparator, Allocator>::compact_map(const Allocator &allocator) :
            compact_map(Comparator(), allocator) {}

    template<class Key, class T, class Comparator, class Allocator>
    compact_map<Key, T, Comparator, Allocator>::compact_map(const Comparator &comparator, const Allocator &allocator) :
            comparator_(comparator),
            vector_(allocator)
    {}

    template<class Key, class T, class Comparator, class Allocator>
    std::pair<int, bool> compact_map<Key, T, Comparator, Allocator>::dichotomy_search(const Key &key, int min, int max) const {
        const int diff = max - min;
        if (diff == 0) return { min, false };

        const int middle = min + diff / 2;

        if (comparator_(key, vector_[middle].first))
            return dichotomy_search(key, min, middle);
        else if (comparator_(vector_[middle].first, key))
            return dichotomy_search(key, middle + 1, max);
        else
            return { middle, true };
    }

    template<class Key, class T, class Comparator, class Allocator>
    typename compact_map<Key, T, Comparator, Allocator>::iterator
    compact_map<Key, T, Comparator, Allocator>::find(const Key &key) {
        const auto pair = dichotomy_search(key, 0, size());
        return pair.second ? begin() + pair.first : end();
    }

    template<class Key, class T, class Comparator, class Allocator>
    typename compact_map<Key, T, Comparator, Allocator>::const_iterator
    compact_map<Key, T, Comparator, Allocator>::find(const Key &key) const {
        const auto pair = dichotomy_search(key, 0, size());
        return pair.second ? cbegin() + pair.first : cend();
    }

    template<class Key, class T, class Comparator, class Allocator>
    T& compact_map<Key, T, Comparator, Allocator>::operator[](const Key &key) {
        const auto pair = dichotomy_search(key, 0, size());
        if (pair.second) return vector_[pair.first].second;

        const auto itPair = insert({ key, {} });
        return itPair.first->second;
    }

    template<class Key, class T, class Comparator, class Allocator>
    T &compact_map<Key, T, Comparator, Allocator>::at(const Key &key) {
        return const_cast<compact_map const*>(this)->at(key);
    }

    template<class Key, class T, class Comparator, class Allocator>
    T const &compact_map<Key, T, Comparator, Allocator>::at(const Key &key) const {
        const auto pair = dichotomy_search(key, 0, size());
        if (pair.second) return vector_[pair.first].first;
        throw std::out_of_range("Key not found in compact_map");
    }

    template<class Key, class T, class Comparator, class Allocator>
    std::pair<typename compact_map<Key, T, Comparator, Allocator>::iterator, bool>
    compact_map<Key, T, Comparator, Allocator>::insert(value_type const& val) {
        const auto pair = dichotomy_search(val.first, 0, size());
        if (pair.second) return { begin() + pair.first, false };

        const int position = comparator_(vector_[pair.first].first, val.first) ?
             pair.first + 1 : pair.first;

        int i = size();
        vector_.emplace_back();
        while (i-- != position) {
            reinterpret_cast<mut_value_type&>(vector_[i + 1]) = std::move(reinterpret_cast<mut_value_type&>(vector_[i]));
        }
        reinterpret_cast<mut_value_type&>(vector_[position]) = reinterpret_cast<mut_value_type const&>(val);
        return { begin() + position, true };
    }

    template<class Key, class T, class Comparator, class Allocator>
    typename compact_map<Key, T, Comparator, Allocator>::iterator
    compact_map<Key, T, Comparator, Allocator>::erase(iterator it) {
        auto i = static_cast<int>(it - begin());
        const int size_ = size();
        while (++i != size_) {
            reinterpret_cast<mut_value_type&>(vector_[i - 1]) = std::move(reinterpret_cast<mut_value_type&>(vector_[i]));
        }
        vector_.pop_back();
        return it;
    }

}
