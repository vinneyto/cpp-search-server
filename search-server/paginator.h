#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

template <typename Iterator>
class IteratorRange {
   public:
    explicit IteratorRange(Iterator begin, Iterator end, size_t size)
        : begin_(begin), end_(end), size_(size) {}

    Iterator begin() const { return begin_; }

    Iterator end() const { return end_; }

    size_t size() const { return size_; }

   private:
    Iterator begin_;
    Iterator end_;
    size_t size_;
};

template <typename Iterator>
class Paginator {
   public:
    explicit Paginator(Iterator begin, Iterator end, size_t page_size) {
        Iterator it = begin;

        while (it != end) {
            Iterator p_begin = it;

            size_t size = std::min(page_size, size_t(distance(it, end)));

            advance(it, size);

            pages_.push_back(IteratorRange(p_begin, it, size));
        }
    }

    auto begin() const { return pages_.begin(); }

    auto end() const { return pages_.end(); }

   private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}