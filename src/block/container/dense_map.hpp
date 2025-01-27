//
// Created by Ninter6 on 2024/11/25.
//

#pragma once

#include <vector>
#include <cassert>

#include "compressed_pair.hpp"

namespace detail {
template <class T, class Deref, class Res = std::invoke_result_t<Deref, typename std::vector<T>::iterator>>
class dense_map_it_warp {
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_reference_t<Res>;
    using pointer = value_type*;
    using reference = value_type&;

    explicit dense_map_it_warp(typename std::vector<T>::iterator it) : it_(it) {}

    operator dense_map_it_warp<T, Deref, std::add_const_t<value_type>&>() const {
        return dense_map_it_warp<T, Deref, std::add_const_t<value_type>&>{it_};
    }

    Res operator*() const { static Deref d{}; return d(it_); }

    Res operator[](size_t i) const { return *(*this + i); }

    pointer operator->() const { return &**this; }

    dense_map_it_warp& operator++() { return ++it_, *this; }

    dense_map_it_warp operator++(int) {
        dense_map_it_warp tmp = *this;
        return ++(*this), tmp;
    }

    dense_map_it_warp& operator--() { return --it_, *this; }

    dense_map_it_warp operator--(int) {
        dense_map_it_warp tmp = *this;
        return --(*this), tmp;
    }

    dense_map_it_warp& operator+=(difference_type n) {
        it_ += n;
        return *this;
    }

    dense_map_it_warp operator+(difference_type n) const {
        return dense_map_it_warp(it_ + n);
    }

    dense_map_it_warp& operator-=(difference_type n) {
        it_ -= n;
        return *this;
    }

    dense_map_it_warp operator-(difference_type n) const {
        return dense_map_it_warp(it_ - n);
    }

    difference_type operator-(const dense_map_it_warp& other) const {
        return it_ - other.it_;
    }

    bool operator==(const dense_map_it_warp& other) const {
        return it_ == other.it_;
    }

    bool operator!=(const dense_map_it_warp& other) const {
        return it_ != other.it_;
    }

    bool operator<(const dense_map_it_warp& other) const {
        return it_ < other.it_;
    }

    bool operator<=(const dense_map_it_warp& other) const {
        return it_ <= other.it_;
    }

    bool operator>(const dense_map_it_warp& other) const {
        return it_ > other.it_;
    }

    bool operator>=(const dense_map_it_warp& other) const {
        return it_ >= other.it_;
    }

private:
    typename std::vector<T>::iterator it_;
};
} // namespace detail

template <class Key, class Value, class Hash = std::hash<Key>, class Eq = std::equal_to<Key>, class Alloc = std::allocator<std::pair<const Key, Value>>>
class dense_map {
    static constexpr size_t null_index = SIZE_MAX;
    static constexpr float hash_k = .707f;

    using value_t = std::pair<const Key, Value>;

    struct node_t {
        template <class...Args>
        explicit node_t(Args&&...args) : value{std::forward<Args>(args)...} {}
        value_t value;
        size_t next = null_index;
    };

    using alloc_traits = std::allocator_traits<Alloc>;
    using sparse_container = std::vector<size_t, typename alloc_traits::template rebind_alloc<size_t>>;
    using packed_container = std::vector<node_t, typename alloc_traits::template rebind_alloc<node_t>>;

    struct node_deref { value_t& operator()(const typename packed_container::iterator& it) const {
        return it->value;
    }};

    [[nodiscard]] auto index_to_iterator(size_t index)  { return begin() + index; }
    [[nodiscard]] auto index_to_iterator(size_t index) const { return cbegin() + index; }

    [[nodiscard]] size_t hash_of(size_t i) const { return sparse_.second()(packed_.first()[i].value.first); }
    void set_hash(size_t hash, size_t index) {
        assert(sparse_.first().size() > 0);
        hash %= sparse_.first().size();
        sparse_.first()[hash] = index;
    }

    template <class...Args>
    void emplace_back(Args&&...args) {
        packed_.first().emplace_back(std::forward<Args>(args)...);
        length_++;
    }
    template <class...Args>
    void replace_back(Args&&...args) {
        auto* node = &packed_.first()[length_++];
        node->~node_t();
        new (node) node_t(std::forward<Args>(args)...);
    }

    [[nodiscard]] size_t hash_list_back(size_t hash) const {
        hash %= sparse_.first().size();
        size_t back = sparse_.first()[hash];
        if (back != null_index)
            while (packed_.first()[back].next != null_index)
                back = packed_.first()[back].next;
        return back;
    }
    [[nodiscard]] size_t hash_list_pre(size_t index) const {
        auto hash = hash_of(index) % sparse_.first().size();
        size_t pre = sparse_.first()[hash];
        if (pre != index)
            while (packed_.first()[pre].next != index)
                assert(pre != null_index), pre = packed_.first()[pre].next;
        return pre;
    }

    void rehash() {
        sparse_.first().assign(static_cast<size_t>(length_ * 2 / hash_k), null_index);
        for (size_t i = 0; i < length_; i++)
            sparse_emplace(i);
    }

    // need: false, needn't: true
    bool rehash_if_need() {
        if (length_ < sparse_.first().size() * hash_k)
            return true;
        rehash();
        return false;
    }

    void sparse_emplace(size_t id) {
        auto hs = hash_of(id);
        if (auto back = hash_list_back(hs); back == null_index) {
            set_hash(hs, id);
            packed_.first()[id].next = null_index;
        } else {
            packed_.first()[back].next = id;
        }
    }

    void isolate(size_t index) {
        auto pre = hash_list_pre(index);
        assert(pre != null_index);
        if (pre == index)
            set_hash(hash_of(index), packed_.first()[index].next);
        else
            packed_.first()[pre].next = packed_.first()[index].next;
        packed_.first()[index].next = null_index;
    }

    void swap_only(size_t index) {
        isolate(index);
        if (index != --length_) {
            auto pre = hash_list_pre(length_);
            assert(pre != null_index);
            if (pre == length_) set_hash(hash_of(length_), index);
            else packed_.first()[pre].next = index;
//            std::swap(packed_.first()[index], packed_.first()[length_]);
            auto pa = &packed_.first()[index], pb = &packed_.first()[length_];
            auto temp = std::move(*pa);
            pa->~node_t();
            new (pa) node_t{std::move(*pb)};
            pb->~node_t();
            new (pb) node_t{std::move(temp)};
        }
    }

    [[nodiscard]] size_t find_index(const Key& key) const {
        if (sparse_.first().empty()) return null_index;
        auto hs = sparse_.second()(key) % sparse_.first().size();
        auto i = sparse_.first()[hs];
        while (i != null_index && !packed_.second()(packed_.first()[i].value.first, key))
            i = packed_.first()[i].next;
        return i;
    }

public:
    using allocator_type = Alloc;
    using key_type = Key;
    using mapped_type = Value;
    using value_type = value_t;
    using iterator = detail::dense_map_it_warp<node_t, node_deref>;
    using const_iterator = detail::dense_map_it_warp<node_t, node_deref, const value_t&>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    dense_map() = default;

    [[nodiscard]] size_t size() const { return length_; }
    [[nodiscard]] size_t free_size() const { return packed_.first().size() - length_; }
    [[nodiscard]] bool empty() const { return length_ == 0; }

    [[nodiscard]] size_t capacity() const { return packed_.first().capacity(); }
    void reserve(size_t n) { packed_.first().reserve(n); }

    template <class...Args>
    iterator emplace(Args&&...args) {
        size_t index = length_;
        if (free_size() == 0) {
            emplace_back(std::forward<Args>(args)...);
        } else {
            replace_back(std::forward<Args>(args)...);
        }

        if (rehash_if_need())
            sparse_emplace(index);

        return index_to_iterator(index);
    }

    iterator push(const value_t& v) { return emplace(v); }
    iterator push(const Key& key) { return emplace(std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>{}); }

    void erase(iterator it) {
        auto index = std::distance(begin(), it);
        assert(index < length_);
        swap_only(index);
    }

    void erase(iterator first, iterator last) {
        assert(first <= last);
        while (last != first) erase(--last);
    }

    void erase(const Key& key) {
        if (auto it = find(key); it != end())
            erase(it);
    }

    void pop(const Key& key) {
        erase(key);
    }

    void pop_back() { erase(index_to_iterator(length_ - 1)); }

    void clear() {
        std::uninitialized_fill(sparse_.first().begin(), sparse_.first().end(), null_index);
        length_ = 0;
    }

    void free_clear() {
        packed_.first().erase(packed_.first().begin() + length_, packed_.first().end());
    }

    void shrink_to_fit() {
        free_clear();
        packed_.first().shrink_to_fit();
    }

    const value_t& undo_pop() {
        assert(free_size() > 0);
        sparse_emplace(length_++);
        return back();
    }

    [[nodiscard]] iterator find(const Key& key) {
        auto i = find_index(key);
        return i == null_index ? end() : index_to_iterator(i);
    }
    [[nodiscard]] const_iterator find(const Key& key) const {
        auto i = find_index(key);
        return i == null_index ? end() : index_to_iterator(i);
    }

    Value& at(const Key& key) {
        auto i = find(key);
        assert(i != end());
        return i->second;
    }
    const Value& at(const Key& key) const {
        auto i = find(key);
        assert(i != end());
        return i->second;
    }

    Value& operator[](const Key& key) {
        if (auto it = find(key); it != end())
            return it->second;
        return push(key)->second;
    }
    const Value& operator[](const Key& key) const {
        return at(key);
    }

    [[nodiscard]] bool contains(const Key& key) const { return find(key) != end(); }

    void rotate_pop(iterator it) {
        auto index = std::distance(begin(), it);
        isolate(index);
        for (auto&& i : sparse_.first())
            if (i != null_index && i > index)
                --i;
        for (auto p = &packed_.first()[index] + 1, e = packed_.first().data() + length_--; p < e; ++p) {
            auto tmp = std::move(p[-1]);
            p[-1].~node_t();
            new (p-1) node_t{std::move(*p)};
            p->~node_t();
            new (p) node_t{std::move(tmp)};
        }
    }

    void rotate_pop(const Key& key) {
        auto it = find(key);
        if (it != end())  rotate_pop(it);
    }

    [[nodiscard]] value_t& front() { return packed_.first().front().value; }
    [[nodiscard]] value_t& back() { return packed_.first()[length_ - 1].value; }
    [[nodiscard]] const value_t& front() const { return packed_.first().front().value; }
    [[nodiscard]] const value_t& back() const { return packed_.first()[length_ - 1].value; }

    [[nodiscard]] iterator begin() { return iterator{packed_.first().begin()}; }
    [[nodiscard]] iterator end() { return begin() + length_; }
    [[nodiscard]] const_iterator begin() const { return const_iterator{ const_cast<packed_container&>(packed_.first()).begin()}; }
    [[nodiscard]] const_iterator end() const { return begin() + length_; }
    [[nodiscard]] auto rbegin() { return std::make_reverse_iterator(end()); }
    [[nodiscard]] auto rend() { return std::make_reverse_iterator(begin()); }
    [[nodiscard]] auto rbegin() const { return std::make_reverse_iterator(end()); }
    [[nodiscard]] auto rend() const { return std::make_reverse_iterator(begin()); }
    [[nodiscard]] const_iterator cbegin() const { return begin(); }
    [[nodiscard]] const_iterator cend() const { return end(); }
    [[nodiscard]] auto crbegin() const { return rbegin(); }
    [[nodiscard]] auto crend() const { return rend(); }

private:
    compressed_pair<sparse_container, Hash> sparse_;
    compressed_pair<packed_container, Eq>   packed_;
    size_t length_{};
};
