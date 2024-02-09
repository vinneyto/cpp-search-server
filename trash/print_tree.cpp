#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

namespace fs = std::filesystem;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

void PrintTreeRecursively(ostream& dst, const path& dir_path, const string& prefix) {
    vector<fs::directory_entry> entries;

    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.exists()) {
            entries.push_back(entry);
        }
    }

    sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        return a.path().filename().string() > b.path().filename().string();
    });

    for (const auto& entry : entries) {
        dst << prefix << entry.path().filename().string() << '\n';
        if (entry.is_directory()) {
            PrintTreeRecursively(dst, entry.path(), prefix + "  ");
        }
    }
}

void PrintTree(std::ostream& dst, const fs::path& p) {
    if (!fs::exists(p) || !fs::is_directory(p)) {
        throw std::runtime_error("Path does not exist or is not a directory");
    }
    dst << p.filename().string() << '\n';
    PrintTreeRecursively(dst, p, "  ");
}

int main() {
    error_code err;
    filesystem::remove_all("test_dir", err);
    filesystem::create_directories("test_dir"_p / "a"_p, err);
    filesystem::create_directories("test_dir"_p / "b"_p, err);

    ofstream("test_dir"_p / "b"_p / "f1.txt"_p);
    ofstream("test_dir"_p / "b"_p / "f2.txt"_p);
    ofstream("test_dir"_p / "a"_p / "f3.txt"_p);

    ostringstream out;
    PrintTree(out, "test_dir"_p);
    assert(out.str() ==
           "test_dir\n"
           "  b\n"
           "    f2.txt\n"
           "    f1.txt\n"
           "  a\n"
           "    f3.txt\n"s);
}