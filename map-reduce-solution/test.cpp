#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <cstdio>

using namespace std;


struct Pair {
    string key;
    int value;
};

class MapFunction {
public:
    vector<Pair> operator()(const string& text) const {
        vector<Pair> pairs;
        stringstream ss(text);
        string word;
        while (ss >> word) {
            pairs.push_back({ word, 1 });
        }
        return pairs;
    }
};

class ReduceFunction {
public:
    map<string, int> operator()(const vector<Pair>& pairs) const {
        map<string, int> reducedData;
        for (const auto& pair : pairs) {
            reducedData[pair.key] += pair.value;
        }
        return reducedData;
    }
};

void processFilePart(const string& filename, int partNumber, MapFunction mapFunction) {
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    vector<Pair> pairs = mapFunction(text);

    ofstream outputFile("localResult" + to_string(partNumber) + ".txt");
    for (const auto& pair : pairs) {
        outputFile << pair.key << " " << pair.value << '\n';
    }
}

int splitFile(const string& filename) {
    ifstream file(filename);
    string line, paragraph;
    int fileCount = 0;
    int paragraphCount = 0;
    FILE* size = fopen(filename.c_str(), "rb");
    long filesize = 0;
    if (size != nullptr) {
        // 파일 포인터를 파일 끝으로 이동하여 크기 구하기
        std::fseek(size, 0, SEEK_END);
        filesize = std::ftell(size);

        std::fclose(size);

        if (filesize != -1) {
            std::cout << "File size: " << filesize << " bytes" << std::endl;
        }
        else {
            std::cerr << "Error getting file size." << std::endl;
        }
    }
    else {
        std::cerr << "Error opening file." << std::endl;
    }

    // 파일을 4개로 나누어 새로운 파일 생성
    ifstream inputFile(filename);

    long chunkSize = filesize / 4; // 4개로 등분

    for (int i = 0; i < 4; ++i) {
        fileCount++;
        ofstream out("part" + to_string(fileCount) + ".txt");

        long bytesRead = 0;
        while (getline(inputFile, line)) {
            bytesRead += line.length() + 1; // 길이와 개행 문자에 대한 공간
            paragraph += line + "\n";

            if (bytesRead >= chunkSize)
                break;
        }

        out << paragraph;
        paragraph.clear();
    }

    inputFile.close();

    return fileCount;
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();
    int fileCount = splitFile("text.txt");

    vector<thread> threads;
    MapFunction mapFunction;

    for (int i = 1; i <= fileCount; ++i) {
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i, mapFunction));
    }

    for (auto& t : threads) {
        t.join();
    }

    ReduceFunction reduceFunction;
    map<string, int> finalResult;
    for (int i = 1; i <= fileCount; ++i) {
        ifstream localFile("localResult" + to_string(i) + ".txt");
        string key;
        int value;
        vector<Pair> pairs;
        while (localFile >> key >> value) {
            pairs.push_back({ key, value });
        }
        map<string, int> reducedData = reduceFunction(pairs);
        for (const auto& pair : reducedData) {
            finalResult[pair.first] += pair.second;
        }
    }
    auto end_time = chrono::high_resolution_clock::now();

    for (const auto& pair : finalResult) {
        cout << pair.first << ": " << pair.second << '\n';
    }

    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms\n";

    return 0;
}
