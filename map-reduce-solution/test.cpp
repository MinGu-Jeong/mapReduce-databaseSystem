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

struct Pair
{
    string key;
    int value;
};

vector<Pair> mapFunction(const string &text)
{
    vector<Pair> pairs;
    stringstream ss(text);
    string word;
    while (ss >> word)
    {
        pairs.push_back({word, 1});
    }
    return pairs;
}

map<string, int> reduce(const vector<Pair> &pairs)
{
    map<string, int> reducedData;
    for (const auto &pair : pairs)
    {
        reducedData[pair.key] += pair.value;
    }
    return reducedData;
}

void processFilePart(const string &filename, int partNumber)
{
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    vector<Pair> pairs = mapFunction(text);

    // 결과를 로컬 파일에 저장
    ofstream outputFile("localResult" + to_string(partNumber) + ".txt");
    for (const auto &pair : pairs)
    {
        outputFile << pair.key << " " << pair.value << '\n';
    }
}

int splitFile(const string &filename)
{
    ifstream file(filename);
    string line, paragraph;
    int fileCount = 0;
    int paragraphCount = 0;
    while (getline(file, line))
    {
        if (line.empty())
        { // 문단이 끝났다고 가정
            paragraphCount++;
            if (paragraphCount == 1540) // n개 문단마다 파일을 분할
            {
                ofstream out("part" + to_string(++fileCount) + ".txt");
                out << paragraph;
                paragraph.clear();
                paragraphCount = 0;
            }
        }
        else
        {
            paragraph += line + "\n";
        }
    }

    // 마지막 문단 처리
    if (!paragraph.empty())
    {
        ofstream out("part" + to_string(++fileCount) + ".txt");
        out << paragraph;
    }

    return fileCount;
}

int main()
{
    auto start_time = chrono::high_resolution_clock::now();
    int fileCount = splitFile("text.txt");

    vector<thread> threads;

    for (int i = 1; i <= fileCount; ++i)
    {
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    // 여기에서 모든 로컬 결과 파일을 병합, 정렬 및 reduce 처리
    map<string, int> finalResult;
    for (int i = 1; i <= fileCount; ++i)
    {
        ifstream localFile("localResult" + to_string(i) + ".txt");
        string key;
        int value;
        while (localFile >> key >> value)
        {
            finalResult[key] += value;
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    // Sort and Reduce (sort는 map<string, int>에 내장되어 있으므로 필요 없음)

    // 최종 결과 출력
    for (const auto &pair : finalResult)
    {
        cout << pair.first << ": " << pair.second << '\n';
    }

    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms\n";
    return 0;
}