#pragma once

#include <deque>
#include <vector>

#include "document.h"
#include "search_server.h"

class RequestQueue {
   public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentPredicate document_predicate) {
        ++current_time_;

        if (!requests_.empty() &&
            current_time_ - requests_.front().execution_time >= min_in_day_) {
            if (requests_.front().documents.empty()) {
                --no_result_requests_count_;
            }

            requests_.pop_front();
        }

        auto documents =
            search_server_.FindTopDocuments(raw_query, document_predicate);

        if (documents.empty()) {
            ++no_result_requests_count_;
        }

        requests_.push_back({current_time_, documents});

        return documents;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

   private:
    struct QueryResult {
        int execution_time;
        std::vector<Document> documents;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;

    const SearchServer& search_server_;

    int no_result_requests_count_ = 0;

    int current_time_ = 0;
};