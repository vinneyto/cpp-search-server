#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

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

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

class SearchServer {
   public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);

        const double inv_count = 1.0 / words.size();

        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_count;
        }

        document_ratings_[document_id] = ComputeAverageRating(ratings);
        document_status_[document_id] = status;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        matched_documents.erase(remove_if(matched_documents.begin(),
                                          matched_documents.end(),
                                          [status, this](const Document& doc) { return document_status_.at(doc.id) != status; }),
                                matched_documents.end());

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return abs(lhs.relevance - rhs.relevance) < EPSILON ? lhs.rating > rhs.rating : lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        const DocumentStatus status = document_status_.count(document_id) > 0 ? document_status_.at(document_id) : DocumentStatus::BANNED;

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.at(word).count(document_id) > 0) {
                return tuple(vector<string>(), status);
            }
        }

        vector<string> words;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.at(word).count(document_id) > 0) {
                words.push_back(word);
            }
        }

        return tuple(words, status);
    }

    int GetDocumentCount() const {
        return document_status_.size();
    }

   private:
    struct QueryWord {
        string data;
        bool is_minus;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }

        int total = accumulate(ratings.begin(), ratings.end(), 0, [](int a, int b) { return a + b; });

        return total / static_cast<int>(ratings.size());
    }

    map<string, map<int, double>> word_to_document_freqs_;
    map<int, int> document_ratings_;
    map<int, DocumentStatus> document_status_;

    set<string> stop_words_;

    double CalculateIDF(const string& word) const {
        return log(GetDocumentCount() / static_cast<double>(word_to_document_freqs_.at(word).size()));
    }

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus};
    }

    Query ParseQuery(const string& text) const {
        SearchServer::Query query;
        for (string word : SplitIntoWordsNoStop(text)) {
            SearchServer::QueryWord query_word = ParseQueryWord(word);

            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
        return query;
    }

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_to_relevance;

        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) > 0) {
                double idf = CalculateIDF(word);

                for (const auto [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance[id] += tf * idf;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) > 0) {
                for (const auto [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }

        vector<Document> matched_documents;

        for (const auto& [id, relevance] : document_to_relevance) {
            matched_documents.push_back({id, relevance, document_ratings_.at(id)});
        }

        return matched_documents;
    }
};

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});

    for (const Document& document : search_server.FindTopDocuments("ухоженный кот"s)) {
        PrintDocument(document);
    }
}