#include "document.h"

#include "iostream"

using namespace std;

Document::Document(int _id, double _relevance, int _rating)
    : id(_id), relevance(_relevance), rating(_rating) {}