#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
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

template <typename Term>
ostream& operator<<(ostream& os, const vector<Term>& terms) {
    os << "[";
    bool is_first = true;
    for (const string& term : terms) {
        if (is_first) {
            os << term;
            is_first = false;
        } else {
            os << ", "s << term;
        }
    }
    os << "]";
    return os;
}

template <typename Term>
ostream& operator<<(ostream& os, const set<Term>& terms) {
    os << "{";
    bool is_first = true;
    for (const string& term : terms) {
        if (is_first) {
            os << term;
            is_first = false;
        } else {
            os << ", "s << term;
        }
    }
    os << "}";
    return os;
}

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    Document() = default;

    Document(int _id, double _relevance, int _rating)
        : id(_id), relevance(_relevance), rating(_rating) {}
};

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

ostream& operator<<(ostream& os, const Document& document) {
    os << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating << " }"s;
    return os;
}

enum DocumentStatus { ACTUAL,
                      IRRELEVANT,
                      BANNED,
                      REMOVED };

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
ostream& operator<<(ostream& os, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); it++) {
        os << *it;
    }
    return os;
}

template <typename Iterator>
class Paginator {
   public:
    explicit Paginator(Iterator begin, Iterator end, size_t page_size) {
        Iterator it = begin;

        while (it != end) {
            Iterator p_begin = it;

            size_t size = min(page_size, size_t(distance(it, end)));

            advance(it, size);

            pages_.push_back(IteratorRange(p_begin, it, size));
        }
    }

    auto begin() const { return pages_.begin(); }

    auto end() const { return pages_.end(); }

   private:
    vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

class SearchServer {
   public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words) {
        for (const string& word : stop_words) {
            if (!IsValidWord(word)) {
                throw invalid_argument("stop word is invalid: "s + word);
            }
            stop_words_.insert(word);
        }
    }

    explicit SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) {}

    void AddDocument(int document_id, const string& document,
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

    template <typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query,
                                      Predicate predicate) const {
        Query query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query, predicate);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return abs(lhs.relevance - rhs.relevance) < EPSILON
                            ? lhs.rating > rhs.rating
                            : lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query,
                                      DocumentStatus document_status) const {
        return FindTopDocuments(
            raw_query, [document_status](int document_id, DocumentStatus status,
                                         int rating) {
                return status == document_status;
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(
            raw_query, [](int document_id, DocumentStatus status, int rating) {
                return status == DocumentStatus::ACTUAL;
            });
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
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

    int GetDocumentCount() const { return document_status_.size(); }

    int GetDocumentId(int index) const { return document_ids_.at(index); }

   private:
    struct QueryWord {
        string data;
        bool is_minus;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    static bool IsValidWord(const string_view& word) {
        return none_of(word.begin(), word.end(),
                       [](char c) { return c >= '\0' && c < ' '; });  // [0, 32)
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }

        int total = accumulate(ratings.begin(), ratings.end(), 0,
                               [](int a, int b) { return a + b; });

        return total / static_cast<int>(ratings.size());
    }

    map<string, map<int, double>> word_to_document_freqs_;
    map<int, int> document_ratings_;
    map<int, DocumentStatus> document_status_;

    set<string> stop_words_;

    vector<int> document_ids_;

    double CalculateIDF(const string& word) const {
        return log(
            GetDocumentCount() /
            static_cast<double>(word_to_document_freqs_.at(word).size()));
    }

    QueryWord ParseQueryWord(string text) const {
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

    Query ParseQuery(const string& text) const {
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

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
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

    template <typename Predicate>
    vector<Document> FindAllDocuments(const Query& query,
                                      Predicate predicate) const {
        map<int, double> document_to_relevance;

        for (const string& word : query.plus_words) {
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

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(id);
            }
        }

        vector<Document> matched_documents;

        for (const auto& [id, relevance] : document_to_relevance) {
            auto rating = document_ratings_.at(id);

            matched_documents.push_back({id, relevance, rating});
        }

        return matched_documents;
    }
};

class RequestQueue {
   public:
    explicit RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для
    // нашей статистики
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
                                         DocumentStatus status) {
        return AddFindRequest(
            raw_query, [status](int document_id, DocumentStatus document_status,
                                int rating) { return document_status == status; });
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int GetNoResultRequests() const {
        return no_result_requests_count_;
    }

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

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str,
                     const string& u_str, const string& file,
                     const string& func, unsigned line,
                     const string& hint) {
    using namespace std;
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str,
                const string& file, const string& func, unsigned line,
                const string& hint) {
    using namespace std;
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) \
    AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) \
    AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename F>
void RunTestImpl(const F& func, const string& t_str) {
    using namespace std;
    func();
    cerr << t_str << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server({});
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп -
    // слов, возвращает пустой результат
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestExcludeMinusWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    SearchServer server("in the"s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto result = server.FindTopDocuments("cat");
    ASSERT_EQUAL(result.size(), 1);

    auto search_result = server.FindTopDocuments("cat -city"s);

    ASSERT(search_result.empty());
}

void TestRelevanceSorting() {
    SearchServer server({});
    server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(43, "cat in the big city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(44, "big dog"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(45, "perro esta en la ciudad"s, DocumentStatus::ACTUAL,
                       {});

    auto result = server.FindTopDocuments("big cat city"s);

    ASSERT_EQUAL(result.size(), 3);
    ASSERT_EQUAL(result[0].id, 43);
    ASSERT_EQUAL(result[1].id, 42);
    ASSERT_EQUAL(result[2].id, 44);
}

void TestRelevanceCalculation() {
    SearchServer server("in the"s);
    server.AddDocument(42, "cat in the cat city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(43, "big cat"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(44, "little dog"s, DocumentStatus::ACTUAL, {});

    int first_document_words_count = 3;
    int cat_in_first_document_count = 2;
    int cat_in_documents_count = 2;
    int documents_count = 3;

    double tf = cat_in_first_document_count /
                static_cast<double>(first_document_words_count);
    double idf = log(documents_count * 1.0 / cat_in_documents_count);
    double relevane = tf * idf;

    auto result = server.FindTopDocuments("cat"s);

    ASSERT_EQUAL(result.size(), 2);
    ASSERT_EQUAL(result[0].id, 42);
    ASSERT_EQUAL(result[0].relevance, relevane);
}

void TestSearchByPredicate() {
    SearchServer server("in the"s);
    server.AddDocument(42, "cat in the cat city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(43, "big cat"s, DocumentStatus::BANNED, {});
    server.AddDocument(44, "little cat"s, DocumentStatus::REMOVED, {1, 2, 3});

    auto result = server.FindTopDocuments(
        "cat", [](int document_id, DocumentStatus status, int rating) {
            (void)document_id;
            (void)rating;
            return status == DocumentStatus::BANNED;
        });

    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL(result[0].id, 43);

    result = server.FindTopDocuments(
        "cat", [](int document_id, DocumentStatus status, int rating) {
            (void)document_id;
            return status == DocumentStatus::REMOVED &&
                   rating == (1 + 2 + 3) / 3;
        });

    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL(result[0].id, 44);

    result = server.FindTopDocuments(
        "cat", [](int document_id, DocumentStatus status, int rating) {
            (void)status;
            (void)rating;
            return document_id == 42;
        });

    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL(result[0].id, 42);

    result = server.FindTopDocuments(
        "cat", [](int document_id, DocumentStatus status, int rating) {
            (void)document_id;
            return status == DocumentStatus::ACTUAL || rating > 0;
        });

    ASSERT_EQUAL(result.size(), 2);
    ASSERT_EQUAL(result[0].id, 44);
    ASSERT_EQUAL(result[1].id, 42);
}

void TestSearchByStatus() {
    SearchServer server("in the"s);
    server.AddDocument(42, "cat in the cat city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(43, "big cat"s, DocumentStatus::BANNED, {});

    auto result = server.FindTopDocuments("cat", DocumentStatus::BANNED);

    ASSERT_EQUAL(result.size(), 1);
    ASSERT_EQUAL(result[0].id, 43);
}

void TestMatchDocumentReturnActialStatus() {
    SearchServer server({});
    server.AddDocument(42, "gray cat"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(43, "brown cat"s, DocumentStatus::IRRELEVANT, {});
    server.AddDocument(44, "little cat"s, DocumentStatus::BANNED, {});
    server.AddDocument(45, "giant cat"s, DocumentStatus::REMOVED, {});

    vector<string> words;
    DocumentStatus status;

    tie(words, status) = server.MatchDocument("gray", 42);
    ASSERT_EQUAL(static_cast<int>(status),
                 static_cast<int>(DocumentStatus::ACTUAL));

    tie(words, status) = server.MatchDocument("brown", 43);
    ASSERT_EQUAL(static_cast<int>(status),
                 static_cast<int>(DocumentStatus::IRRELEVANT));

    tie(words, status) = server.MatchDocument("little", 44);
    ASSERT_EQUAL(static_cast<int>(status),
                 static_cast<int>(DocumentStatus::BANNED));

    tie(words, status) = server.MatchDocument("giant", 45);
    ASSERT_EQUAL(static_cast<int>(status),
                 static_cast<int>(DocumentStatus::REMOVED));
}

void TestMatchDocumentCheckMinusWords() {
    SearchServer server({});
    server.AddDocument(42, "gray cat"s, DocumentStatus::ACTUAL, {});

    auto [words, _] = server.MatchDocument("gray cat -cat", 42);

    ASSERT(words.empty());
}

void TestAddWithAverageRating() {
    SearchServer server({});
    server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL,
                       {1, 2, 3});

    ASSERT_EQUAL(server.FindTopDocuments("cat")[0].rating, (1 + 2 + 3) / 3);
}

/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
    RUN_TEST(TestRelevanceSorting);
    RUN_TEST(TestRelevanceCalculation);
    RUN_TEST(TestAddWithAverageRating);
    RUN_TEST(TestSearchByPredicate);
    RUN_TEST(TestSearchByStatus);
    RUN_TEST(TestMatchDocumentReturnActialStatus);
    RUN_TEST(TestMatchDocumentCheckMinusWords);
}

int main() {
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("curly dog"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("big collar"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    return 0;
}