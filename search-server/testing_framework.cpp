#include "testing_framework.h"

#include <cmath>

#include "document.h"
#include "search_server.h"

using namespace std;

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