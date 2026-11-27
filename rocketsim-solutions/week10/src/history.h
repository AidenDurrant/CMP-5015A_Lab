// history.h -- Week 10 (Medium 1): a fixed-capacity ring buffer, i.e. a
// "black box" that remembers the last N values of any type T. std::array holds
// the storage, so no destructor or copy members need writing.

#pragma once

#include <array>
#include <cstddef>

template <typename T, std::size_t N>
class History
{
    // Week 10 (Hard 1): with N == 0, push() would compute % 0, which is
    // undefined. Stop it at compile time instead.
    static_assert(N > 0, "History needs room for at least one value");

public:
    void push(const T& value)
    {
        data_[(start_ + count_) % N] = value;
        if (count_ < N)
        {
            ++count_;
        }
        else
        {
            start_ = (start_ + 1) % N;   // overwrite the oldest
        }
    }

    std::size_t size() const { return count_; }
    static constexpr std::size_t capacity() { return N; }

    // Oldest first.
    const T& operator[](std::size_t i) const { return data_[(start_ + i) % N]; }
    const T& latest() const { return data_[(start_ + count_ - 1) % N]; }

private:
    std::array<T, N> data_{};
    std::size_t start_ = 0;
    std::size_t count_ = 0;
};
