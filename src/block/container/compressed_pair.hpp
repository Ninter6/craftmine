//
// Created by Ninter6 on 2024/11/24.
//

#pragma once

#include <utility>

template <class First, class Second>
struct compressed_pair : private First, private Second {
    compressed_pair() = default;
    compressed_pair(const First& f, const Second& s) : First(f), Second(s) {}
    compressed_pair(const First& f, Second&& s) : First(f), Second(std::move(s)) {}
    compressed_pair(First&& f, const Second& s) : First(std::move(f)), Second(s) {}
    compressed_pair(First&& f, Second&& s) : First(std::move(f)), Second(std::move(s)) {}

    [[nodiscard]] First& first() { return *this; }
    [[nodiscard]] const First& first() const { return *this; }
    [[nodiscard]] Second& second() { return *this; }
    [[nodiscard]] const Second& second() const { return *this; }
};
