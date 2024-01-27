#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "document.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
   public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words_text);

    void AddDocument(int document_id, const std::string& document,
                     DocumentStatus status, const std::vector<int>& ratings);

    template <typename Predicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query,
                                           Predicate predicate) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query,
                                           DocumentStatus document_status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
                                                                       int document_id) const;

    int GetDocumentCount() const;

    int GetDocumentId(int index) const;

   private:
    struct QueryWord {
        std::string data;
        bool is_minus;
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    static bool IsValidWord(const std::string& word);

    static int ComputeAverageRating(const std::vector<int>& ratings);

    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, int> document_ratings_;
    std::map<int, DocumentStatus> document_status_;

    std::set<std::string> stop_words_;

    std::vector<int> document_ids_;

    double CalculateIDF(const std::string& word) const;

    QueryWord ParseQueryWord(std::string text) const;

    Query ParseQuery(const std::string& text) const;

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    template <typename Predicate>
    std::vector<Document> FindAllDocuments(const Query& query,
                                           Predicate predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) {
    for (const std::string& word : stop_words) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("stop word is invalid: " + word);
        }
        stop_words_.insert(word);
    }
}

template <typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
                                                     Predicate predicate) const {
    Query query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query, predicate);

    std::sort(matched_documents.begin(), matched_documents.end(),
              [](const Document& lhs, const Document& rhs) {
                  return std::abs(lhs.relevance - rhs.relevance) < EPSILON
                             ? lhs.rating > rhs.rating
                             : lhs.relevance > rhs.relevance;
              });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
                                                     Predicate predicate) const {
    std::map<int, double> document_to_relevance;

    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
            auto rating = document_ratings_.at(id);
            auto status = document_status_.at(id);

            if (predicate(id, status, rating)) {
                document_to_relevance[id] += tf * CalculateIDF(word);
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto& [id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(id);
        }
    }

    std::vector<Document> matched_documents;

    for (const auto& [id, relevance] : document_to_relevance) {
        auto rating = document_ratings_.at(id);

        matched_documents.push_back({id, relevance, rating});
    }

    return matched_documents;
}
