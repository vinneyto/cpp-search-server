#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;

// реализуйте эту функцию:
void CreateFiles(const string& config_file) {
    ifstream file(config_file);
    ofstream output;

    string line;

    while (getline(file, line)) {
        if (!line.size()) {
            continue;
        }
        if (line[0] != '>') {
            if (output.is_open()) {
                output.close();
            }
            output.open(line);
        } else {
            string_view sub(line.data() + 1, line.size() - 1);
            output << sub << "\n";
        }
    }
}

string GetLine(istream& in) {
    string s;
    getline(in, s);
    return s;
}

int main() {
    ofstream("test_config.txt"s) << "a.txt\n"
                                    ">10\n"
                                    ">abc\n"
                                    "b.txt\n"
                                    ">123"s;

    CreateFiles("test_config.txt"s);
    ifstream in_a("a.txt"s);
    assert(GetLine(in_a) == "10"s && GetLine(in_a) == "abc"s);

    ifstream in_b("b.txt"s);
    assert(GetLine(in_b) == "123"s);
}