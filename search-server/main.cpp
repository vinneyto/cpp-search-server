#include <cmath>
#include <string>
#include <vector>

#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "search_server.h"
#include "testing_framework.h"

using namespace std;

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
    TestSearchServer();

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