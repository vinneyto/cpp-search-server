#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

void PrintWarning(const path& include_path, const path& file_path, unsigned int line_num) {
    cout << "unknown include file "s << include_path.filename().string() << " at file "s << file_path.string() << " at line "s << line_num << endl;
}

bool CheckIncludeDirectories(ifstream& include_input, path& include_path, const string& include_file, const vector<path>& paths) {
    for (const path& p : paths) {
        include_path = p / include_file;
        include_input.open(include_path.string());

        if (include_input.is_open()) {
            break;
        }
    }

    return include_input.is_open();
}

bool ProcessFile(ifstream& input, ofstream& output, const path& file_path, const vector<path>& paths) {
    // #include "..."
    static regex file_reg(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    // #include <...>
    static regex lib_reg(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

    string line;
    smatch m;

    unsigned int line_num = 1;

    while (getline(input, line)) {
        if (regex_match(line, m, file_reg)) {
            string include_file = string(m[1]);

            // take file from parent directory
            path include_path = file_path.parent_path() / include_file;
            ifstream include_input(include_path.string());

            // or check in include directories
            if (!include_input.is_open() && !CheckIncludeDirectories(
                                                include_input,
                                                include_path,
                                                include_file,
                                                paths)) {
                PrintWarning(include_path, file_path, line_num);
                return false;
            }

            if (!ProcessFile(include_input, output, include_path, paths)) {
                return false;
            }

        } else if (regex_match(line, m, lib_reg)) {
            path include_path;
            ifstream include_input;
            string include_file = string(m[1]);

            // check in include directories immediately
            if (!CheckIncludeDirectories(include_input, include_path, include_file, paths)) {
                PrintWarning(include_path, file_path, line_num);
                return false;
            }

            if (!ProcessFile(include_input, output, include_path, paths)) {
                return false;
            }
        } else {
            output << line << endl;
        }

        line_num++;
    }

    return true;
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ifstream input(in_file.string());
    if (!input) {
        return false;
    }

    ofstream output(out_file.string());
    if (!output) {
        return false;
    }

    return ProcessFile(input, output, in_file, include_directories);
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                        {"sources"_p / "include1"_p, "sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    // cout << GetFileContents("sources/a.in"s) << endl;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}