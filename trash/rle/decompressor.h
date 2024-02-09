#pragma once

#include <fstream>
#include <iostream>
#include <string>

void CopyBites(std::ifstream& input, std::ofstream& output, int data_size) {
    using namespace std;

    const int buffer_size = 4096;
    char buffer[buffer_size];

    while (data_size > 0) {
        int bytes_to_read = min(buffer_size, data_size);

        input.read(buffer, bytes_to_read);

        int bytes_read = input.gcount();

        output.write(buffer, bytes_read);

        data_size -= bytes_read;

        if (input.eof()) {
            break;
        }
    }
}

inline bool DecodeRLE(const std::string& src_name, const std::string& dst_name) {
    using namespace std;
    using namespace std::literals::string_literals;

    ifstream in_file(src_name, ios::binary);
    if (!in_file) {
        cerr << "Can't open input file"s << endl;
        return false;
    }

    ofstream out_file(dst_name, ios::binary);
    if (!out_file) {
        cerr << "Can't open output file"s << endl;
        return false;
    }

    unsigned char header;

    int result = in_file.get();

    while (result != istream::traits_type::eof()) {
        header = static_cast<unsigned char>(result);

        int block_type = (header & 1);
        int data_size = (header >> 1) + 1;

        if (block_type == 0) {
            CopyBites(in_file, out_file, data_size);
        } else {
            result = in_file.get();

            if (result != istream::traits_type::eof()) {
                string repeated(data_size, static_cast<char>(result));

                out_file << repeated;
            }
        }

        result = in_file.get();
    }

    return true;
}