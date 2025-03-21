#pragma once

#include <iostream>
#include <string>

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str,
                     const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line,
                     const std::string& hint) {
    using namespace std;
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) \
    AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str,
                const std::string& file, const std::string& func, unsigned line,
                const std::string& hint);

#define ASSERT(expr) \
    AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) \
    AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename F>
void RunTestImpl(const F& func, const std::string& t_str) {
    using namespace std;
    func();
    std::cerr << t_str << " OK"s << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)