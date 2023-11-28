#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>

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
    while (getline(file, line)) {
        if (line.empty()) {
            paragraphCount++;
            if (paragraphCount == 1540) {
                ofstream out("part" + to_string(++fileCount) + ".txt");
                out << paragraph;
                paragraph.clear();
                paragraphCount = 0;
            }
        }
        else {
            paragraph += line + "\n";
        }
    }

    if (!paragraph.empty()) {
        ofstream out("part" + to_string(++fileCount) + ".txt");
        out << paragraph;
    }

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
