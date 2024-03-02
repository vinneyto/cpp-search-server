#pragma once

#include <array>
#include <stdexcept>

template <typename T, size_t N>
class StackVector {
   public:
    explicit StackVector(size_t a_size = 0) {
        if (a_size > data_.size()) {
            throw std::invalid_argument("size argument bigger than capacity");
        }
        size_ = a_size;
    }

    T& operator[](size_t index) {
        return data_[index];
    }

    const T& operator[](size_t index) const {
        return data_[index];
    }

    typename std::array<T, N>::iterator begin() {
        return data_.begin();
    }

    typename std::array<T, N>::iterator end() {
        return data_.begin() + size_;
    }

    typename std::array<T, N>::const_iterator begin() const {
        return data_.begin();
    }

    typename std::array<T, N>::const_iterator end() const {
        return data_.begin() + size_;
    }

    size_t Size() const {
        return size_;
    }

    size_t Capacity() const {
        return data_.size();
    }

    void PushBack(const T& value) {
        if (data_.size() == size_) {
            throw std::overflow_error("unable to push - capacity overflow");
        }
        data_[size_] = value;
        ++size_;
    }
    T PopBack() {
        if (size_ == 0) {
            throw std::underflow_error("unable to pop - stack is empty");
        }
        --size_;
        return data_[size_];
    }

   private:
    std::array<T, N> data_;
    size_t size_ = 0;
};
