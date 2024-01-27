#pragma once

#include <iostream>

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    Document() = default;

    Document(int _id, double _relevance, int _rating);
};

enum DocumentStatus { ACTUAL,
                      IRRELEVANT,
                      BANNED,
                      REMOVED };