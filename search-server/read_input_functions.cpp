#include "read_input_functions.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<int> ReadRatings() {
    int n = 0;
    cin >> n;
    vector<int> result;
    for (int i = 0; i < n; i++) {
        int r;
        cin >> r;
        result.push_back(r);
    }
    ReadLine();
    return result;
}

ostream& operator<<(ostream& os, const Document& document) {
    os << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating << " }"s;
    return os;
}