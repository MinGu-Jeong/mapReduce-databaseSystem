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

class Map
{
public:
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
};

class Reduce
{
public:
    int reduceValues(const vector<int> &values)
    {
        int sum = 0;
        for (const auto &value : values)
        {
            sum += value;
        }
        return sum;
    }
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

map<string, int> reduce(const map<string, vector<int>> &mergedData)
{
    map<string, int> finalResult;
    for (const auto &it : mergedData)
    {
        int sum = 0;
        for (const auto &value : it.second)
        {
            sum += value;
        }
        finalResult[it.first] = sum;
    }
    return finalResult;
}

void processFilePart(const string &filename, int partNumber)
{
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    Map mapper;
    vector<Pair> pairs = mapper.mapFunction(text);

    ofstream outputFile("localResult" + to_string(partNumber) + ".txt");
    for (const auto &pair : pairs)
    {
        outputFile << pair.key << " " << pair.value << '\n';
    }
}

// 파일을 스레드 개수로 분할하는 함수
int splitFile(const string& filename, int threadCount)
{
    ifstream file(filename);
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // 각 파티션의 크기 계산 (파일 크기를 스레드 개수로 나눔)
    int partitionSize = fileSize / threadCount;

    int partNumber = 0;
    string partFileName;
    string line, partition;
    int partitionCount = 0;
    while (getline(file, line))
    {
        partition += line + "";
        ++partitionCount;

        // 파티션 크기가 계산된 크기를 넘으면 파일에 쓰고 초기화
        if (partitionCount >= partitionSize)
        {
            // 마지막 단어를 다음 파티션으로 넘기기
            size_t lastSpacePos = partition.find_last_of(' ');
            if (lastSpacePos != string::npos)
            {
                string lastWord = partition.substr(lastSpacePos + 1);
                partition.erase(lastSpacePos);

                partFileName = "part" + to_string(partNumber++) + ".txt";
                ofstream partFile(partFileName);
                partFile << partition;

                // 다음 파티션의 시작에 마지막 단어 추가
                partition = lastWord;
            }
            else
            {
                partFileName = "part" + to_string(partNumber++) + ".txt";
                ofstream partFile(partFileName);
                partFile << partition;
                partition.clear();
            }
            partitionCount = 0;
        }
    }

    // 마지막 파티션 처리
    if (!partition.empty())
    {
        partFileName = "part" + to_string(partNumber++) + ".txt";
        ofstream partFile(partFileName);
        partFile << partition;
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
    for (const auto &pair : mergedData)
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

    // 파일 분할
    int fileCount = splitFile("text.txt", 4);

    // 각 파일 파트 처리
    vector<thread> threads;
    for (int i = 1; i <= fileCount; ++i)
    {
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    // 모든 로컬 결과 파일을 병합
    map<string, vector<int>> mergedData = mergeSort(fileCount);

    // 병합된 데이터를 reduce 처리
    Reduce reducer;
    map<string, int> finalResult;
    for (const auto &it : mergedData)
    {
        finalResult[it.first] = reducer.reduceValues(it.second);
    }

    auto end_time = chrono::high_resolution_clock::now();

    // 최종 결과 출력
    ofstream resultFile("result.txt");
    for (const auto &pair : finalResult)
    {
        resultFile << pair.first << ": " << pair.second << endl;
    }

    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms" << endl;
    return 0;
}