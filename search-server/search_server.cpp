#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "string_processing.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {}

void SearchServer::AddDocument(int document_id, const string& document,
                               DocumentStatus status, const vector<int>& ratings) {
    if (document_id < 0) {
        throw invalid_argument("attempt to add document with negative id");
    }

    if (document_ratings_.count(document_id) > 0) {
        throw invalid_argument("attempt to add document twice");
    }

    const vector<string> words = SplitIntoWordsNoStop(document);

    const double inv_count = 1.0 / words.size();

    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_count;
    }

    document_ratings_[document_id] = ComputeAverageRating(ratings);
    document_status_[document_id] = status;

    document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query,
                                                DocumentStatus document_status) const {
    return FindTopDocuments(
        raw_query, [document_status](int document_id, DocumentStatus status,
                                     int rating) {
            return status == document_status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(
        raw_query, [](int document_id, DocumentStatus status, int rating) {
            return status == DocumentStatus::ACTUAL;
        });
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query,
                                                                  int document_id) const {
    Query query = ParseQuery(raw_query);

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id) > 0) {
            return {
                tuple(vector<string>(), document_status_.at(document_id))};
        }
    }

    vector<string> words;

    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id) > 0) {
            words.push_back(word);
        }
    }

    return {tuple(words, document_status_.at(document_id))};
}

int SearchServer::GetDocumentCount() const { return document_status_.size(); }

int SearchServer::GetDocumentId(int index) const { return document_ids_.at(index); }

bool SearchServer::IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(),
                   [](char c) { return c >= '\0' && c < ' '; });  // [0, 32)
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }

    int total = accumulate(ratings.begin(), ratings.end(), 0,
                           [](int a, int b) { return a + b; });

    return total / static_cast<int>(ratings.size());
}

double SearchServer::CalculateIDF(const string& word) const {
    return log(
        GetDocumentCount() /
        static_cast<double>(word_to_document_freqs_.at(word).size()));
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }

    if (!SearchServer::IsValidWord(text)) {
        throw invalid_argument("word in invalid");
    }

    if (text.empty() || text[0] == '-') {
        throw invalid_argument("word in empty");
    }

    return {text, is_minus};
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (string word : SplitIntoWordsNoStop(text)) {
        QueryWord query_word = ParseQueryWord(word);

        if (query_word.is_minus) {
            query.minus_words.insert(query_word.data);
        } else {
            query.plus_words.insert(query_word.data);
        }
    }
    return query;
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("word is invalid: " + word);
        }

        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}