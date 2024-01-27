#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

// set<int>::const_iterator FindNearestElement(const set<int>& numbers, int border) {
//     if (numbers.empty()) {
//         return numbers.end();
//     }

//     if (border <= *numbers.begin()) {
//         return numbers.begin();
//     }

//     if (border >= *prev(numbers.end())) {
//         return prev(numbers.end());
//     }

//     auto it = numbers.lower_bound(border);

//     if (*it == border) {
//         return it;
//     }

//     auto prev_it = prev(it);

//     if ((border - *prev_it) <= (*it - border)) {
//         return prev_it;
//     }

//     return it;
// }

// template <typename RandomIt>
// pair<RandomIt, RandomIt> FindStartsWith(RandomIt range_begin, RandomIt range_end, char prefix) {
//     string l_str(1, prefix);
//     string r_str(1, static_cast<char>(prefix) + 1);

//     auto l_it = lower_bound(range_begin, range_end, l_str);
//     auto r_it = lower_bound(range_begin, range_end, r_str);

//     return {l_it, r_it};
// }

template <typename RandomIt>
pair<RandomIt, RandomIt> FindStartsWith(RandomIt range_begin, RandomIt range_end, string prefix) {
    string l_str(prefix);
    string r_str(prefix);

    r_str.back()++;

    auto l_it = lower_bound(range_begin, range_end, l_str);
    auto r_it = lower_bound(range_begin, range_end, r_str);

    return {l_it, r_it};
}

int main() {
    const vector<string> sorted_strings = {"moscow", "motovilikha", "murmansk"};
    const auto mo_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "mo");
    for (auto it = mo_result.first; it != mo_result.second; ++it) {
        cout << *it << " ";
    }
    cout << endl;
    const auto mt_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "mt");
    cout << (mt_result.first - begin(sorted_strings)) << " " << (mt_result.second - begin(sorted_strings)) << endl;
    const auto na_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), "na");
    cout << (na_result.first - begin(sorted_strings)) << " " << (na_result.second - begin(sorted_strings)) << endl;
    return 0;

    // const vector<string> sorted_strings = {"moscow", "murmansk", "vologda"};
    // const auto m_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), 'm');
    // for (auto it = m_result.first; it != m_result.second; ++it) {
    //     cout << *it << " ";
    // }
    // cout << endl;
    // const auto p_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), 'p');
    // cout << (p_result.first - begin(sorted_strings)) << " " << (p_result.second - begin(sorted_strings)) << endl;
    // const auto z_result = FindStartsWith(begin(sorted_strings), end(sorted_strings), 'z');
    // cout << (z_result.first - begin(sorted_strings)) << " " << (z_result.second - begin(sorted_strings)) << endl;
    // return 0;

    // set<int> numbers = {1, 4, 6};
    // cout << *FindNearestElement(numbers, 0) << " " << *FindNearestElement(numbers, 3) << " "
    //      << *FindNearestElement(numbers, 5) << " " << *FindNearestElement(numbers, 6) << " "
    //      << *FindNearestElement(numbers, 100) << endl;
    // set<int> empty_set;
    // cout << (FindNearestElement(empty_set, 8) == end(empty_set)) << endl;
    // return 0;
}