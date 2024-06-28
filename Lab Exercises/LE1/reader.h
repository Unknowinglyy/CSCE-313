#include "state.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


class Reader{
private:
    std::ifstream input_file;
public:
    Reader(const std::string& filename);
    ~Reader();

    void readData(std::vector<std::vector<std::string>>& csvRows);
};

Reader::Reader(const std::string& filename) {
    input_file.open(filename);

    if (!input_file.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        throw std::runtime_error("Error opening the file: " + filename);
    }
}

Reader::~Reader() {
    input_file.close();
}

void Reader::readData(std::vector<std::vector<std::string>>& csvRows) {
    std::string line;
    int count = 0;
    while (getline(input_file, line)) {
        count++;
        std::istringstream iss(line);
        std::vector<std::string> row;
        std::string value;
        while ( getline(iss, value, ',')) {
            row.push_back(value);
        }
        csvRows.push_back(row);
    }
}