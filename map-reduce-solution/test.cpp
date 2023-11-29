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

vector<Pair> mapFunction(const string& text)
{
    vector<Pair> pairs;
    stringstream ss(text);
    string word;
    while (ss >> word)
    {
        pairs.push_back({ word, 1 });
    }
    return pairs;
}

map<string, int> reduce(const map<string, vector<int>>& mergedData)
{
    map<string, int> finalResult;
    for (const auto& it : mergedData)
    {
        int sum = 0;
        for (const auto& value : it.second)
        {
            sum += value;
        }
        finalResult[it.first] = sum;
    }
    return finalResult;
}


void processFilePart(const string& filename, int partNumber)
{
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    vector<Pair> pairs = mapFunction(text);

    // 결과를 로컬 파일에 저장
    ofstream outputFile("localResult" + to_string(partNumber) + ".txt");
    for (const auto& pair : pairs)
    {
        outputFile << pair.key << " " << pair.value << '\n';
    }
}

int splitFile(const string& filename, streamsize partSize)
{
    ifstream file(filename, ios::binary | ios::ate);
    const auto fileSize = file.tellg();
    file.seekg(0);

    int partNumber = 0;
    string partFileName;
    for (streamoff i = 0; i < fileSize; i += partSize)
    {
        partFileName = "part" + to_string(partNumber++) + ".txt";
        ofstream partFile(partFileName, ios::binary);

        if (i + partSize < fileSize)
        {
            vector<char> buffer(partSize);
            file.read(buffer.data(), partSize);
            partFile.write(buffer.data(), partSize);
        }
        else
        {
            // 마지막 부분 파일의 크기가 partSize보다 작을 수 있음
            vector<char> buffer(fileSize - i);
            file.read(buffer.data(), fileSize - i);
            partFile.write(buffer.data(), fileSize - i);
        }
    }
    return partNumber;
}

map<string, vector<int>> mergeSort(int fileCount)
{
    map<string, vector<int>> mergedData;
    for (int i = 1; i <= fileCount; ++i)
    {
        ifstream localFile("localResult" + to_string(i) + ".txt");
        string key;
        int value;
        while (localFile >> key >> value)
        {
            mergedData[key].push_back(value);
        }
    }

    // 중간 결과를 파일에 저장
    ofstream mergeFile("mergeSort.txt");
    for (const auto& pair : mergedData)
    {
        mergeFile << pair.first << ": [";
        for (int i = 0; i < pair.second.size(); ++i)
        {
            mergeFile << pair.second[i];
            if (i != pair.second.size() - 1)
            {
                mergeFile << ",";
            }
        }
        mergeFile << "]" << endl;
    }

    return mergedData;
}

int main()
{
    auto start_time = chrono::high_resolution_clock::now();
    int fileCount = splitFile("text.txt", 1024 * 1024);  // 파일을 1MB 크기로 분할

    vector<thread> threads;

    for (int i = 1; i <= fileCount; ++i)
    {
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // 모든 로컬 결과 파일을 병합, mergeSort
    map<string, vector<int>> mergedData = mergeSort(fileCount);

    // 병합된 데이터를 reduce 처리
    map<string, int> finalResult = reduce(mergedData);

    auto end_time = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms" << endl;
        return 0;
}
