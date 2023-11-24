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

void processFilePart(const string &filename, mutex &outputMutex, map<string, int> &finalResult)
{
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    vector<Pair> pairs = mapFunction(text);
    sort(pairs.begin(), pairs.end(), [](const Pair &a, const Pair &b)
         { return a.key < b.key; });
    map<string, int> reducedData = reduce(pairs);

    lock_guard<mutex> lock(outputMutex);
    for (const auto &pair : reducedData)
    {
        finalResult[pair.first] += pair.second;
    }
}

void splitFile(const string &filename)
{
    ifstream file(filename);
    string line, paragraph;
    int fileCount = 0;
    while (getline(file, line))
    {
        if (line.empty())
        { // 문단이 끝났다고 가정
            ofstream out("part" + to_string(++fileCount) + ".txt");
            out << paragraph;
            paragraph.clear();
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
}

int main()
{
    splitFile("text.txt"); // 파일 분할

    vector<thread> threads;
    mutex outputMutex;
    map<string, int> finalResult;

    // 예시: 파일을 6개로 분할했다고 가정
    const int numberOfParts = 6; // 실제 분할된 파일 수에 맞춰 조정
    for (int i = 1; i <= numberOfParts; ++i)
    {
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", ref(outputMutex), ref(finalResult)));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    // 최종 결과 출력
    for (const auto &pair : finalResult)
    {
        cout << pair.first << ": " << pair.second << '\n';
    }

    return 0;
}
