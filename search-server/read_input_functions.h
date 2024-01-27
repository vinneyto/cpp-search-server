#pragma once

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "document.h"
#include "paginator.h"

std::string ReadLine();

int ReadLineWithNumber();

std::vector<int> ReadRatings();

std::ostream& operator<<(std::ostream& os, const Document& document);

template <typename Term>
std::ostream& operator<<(std::ostream& os, const std::vector<Term>& terms) {
    os << "[";
    bool is_first = true;
    for (const std::string& term : terms) {
        if (is_first) {
            os << term;
            is_first = false;
        } else {
            os << ", " << term;
        }
    }
    os << "]";
    return os;
}

template <typename Term>
std::ostream& operator<<(std::ostream& os, const std::set<Term>& terms) {
    os << "{";
    bool is_first = true;
    for (const std::string& term : terms) {
        if (is_first) {
            os << term;
            is_first = false;
        } else {
            os << ", " << term;
        }
    }
    os << "}";
    return os;
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); it++) {
        os << *it;
    }
    return os;
}