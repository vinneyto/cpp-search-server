#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

struct ReserveProxyObj {
    size_t capacity;
};

template <typename Type>
class SimpleVector {
   public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) { Resize(size); }

    explicit SimpleVector(ReserveProxyObj capacity_obj) {
        Reserve(capacity_obj.capacity);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> items(size);

        std::fill(items.Get(), items.Get() + size, value);

        items_.swap(items);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        size_t size = std::distance(init.begin(), init.end());

        ArrayPtr<Type> items(size);

        std::copy(init.begin(), init.end(), items.Get());

        items_.swap(items);
        size_ = size;
        capacity_ = size;
    }

    SimpleVector(const SimpleVector& other) {
        if (!other.IsEmpty()) {
            ArrayPtr<Type> items(other.GetSize());

            std::copy(other.begin(), other.end(), items.Get());

            size_ = other.GetSize();
            capacity_ = other.GetCapacity();
            items_.swap(items);
        }
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        // Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept { return size_ == 0; }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);

        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);

        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept { Resize(0); }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            ArrayPtr<Type> items_resized(new_size);

            if (items_.Get() != nullptr) {
                std::copy(begin(), end(), items_resized.Get());
            }

            std::generate(items_resized.Get() + size_,
                          items_resized.Get() + new_size,
                          []() { return Type(); });

            items_.swap(items_resized);

            capacity_ = new_size;
        } else if (new_size > size_) {
            std::generate(begin(), begin() + new_size, []() { return Type(); });
        }

        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);

            if (!IsEmpty()) {
                std::copy(begin(), end(), new_items.Get());
            }

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    void PopBack() noexcept {
        assert(!IsEmpty());

        --size_;
    };

    void PushBack(const Type& value) {
        Expand();

        items_[size_] = value;
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        // assertion is inside PrepareInsert

        size_t index = PrepareInsert(pos);

        items_[index] = value;
        ++size_;

        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin());
        assert(pos <= end());

        assert(!IsEmpty());

        size_t index = pos - items_.Get();
        std::copy(begin() + index + 1, end(), begin() + index);

        --size_;
        return begin() + index;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);

        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs) {
            SimpleVector<Type> tmp(rhs);

            swap(tmp);
        }

        return *this;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept { return items_.Get(); }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept { return items_.Get() + size_; }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept { return items_.Get(); }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept { return items_.Get() + size_; }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept { return begin(); }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept { return end(); }

   private:
    void Expand() {
        if (capacity_ == size_) {
            Reserve(size_ != 0 ? size_ * 2 : 10);
        }
    }

    size_t PrepareInsert(ConstIterator pos) {
        assert(pos >= begin());
        assert(pos <= end());

        size_t index = pos - items_.Get();

        Expand();

        std::copy_backward(begin() + index, end(), end() + 1);

        return index;
    }

    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs,
                       const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs,
                       const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs,
                      const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity) { return {capacity}; }