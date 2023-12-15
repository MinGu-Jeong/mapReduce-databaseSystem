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
        vector<Pair> pairs; // 단어와 빈도수를 저장하는 Pair 벡터
        stringstream ss(text); // 텍스트를 stringstream에 저장하여 단어 단위로 읽기 위해 설정
        string word;

        // 스트림에서 공백으로 구분된 단어를 읽어 Pair 벡터에 추가
        while (ss >> word) // ss에서 단어를 읽어 word에 저장
        {
            pairs.push_back({word, 1}); // 읽어온 word와 1을 Pair로 묶어 벡터에 추가
        }
        return pairs;
    }
};

class Reduce
{
public:
// 주어진 벡터의 합을 계산하는 함수
    int reduceValues(const vector<int> &values)
    {
        int sum = 0;

        // 벡터의 각 요소를 더해 전체 합을 계산
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


// 주어진 파일의 일부를 처리하 로컬 결과 파일에 저장하는 함수
void processFilePart(const string &filename, int partNumber)
{
    ifstream file(filename); // 파일 열기

    // 파일 내용을 문자열로 읽어오기
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    Map mapper;
    // mapFunction을 호출하여 단어와 빈도수의 Pair 벡터 얻기
    vector<Pair> pairs = mapper.mapFunction(text);

// 로컬 결과 파일 생성
    ofstream outputFile("localResult" + to_string(partNumber) + ".txt");
    for (const auto &pair : pairs)
    {
        outputFile << pair.key << " " << pair.value << '\n'; // Pair 정보를 파일에 쓰기
    }
}

// 파일을 문단 수로 분할하는 함수
int splitFileByParagraphs(const string &filename, int numParagraphs)
{
    ifstream file(filename);
    string line, paragraph; // 현재 읽은 라인, 현재까지 문단
    int fileCount = 0; // 생성된 파일의 수
    int paragraphCount = 0; // 현재까지 읽은 문단의 수
    while (getline(file, line))
    {
        if (line.empty() || line == "\r")
        { // 빈 라인 또는 개행 문자만 있는 라인은 문단이 끝났다고 판단
            paragraphCount++;
            if (paragraphCount == numParagraphs) // 사용자 입력 문단마다 파일을 분할
            {
                ofstream out("part" + to_string(++fileCount) + ".txt"); // 분산 저장된 파일 이름
                out << paragraph; // 현재까지 읽은 문단을 파일에 쓰기
                paragraph.clear(); // 읽은 문단 초기화
                paragraphCount = 0; // 읽은 문단 수 초기화
            }
        }
        else
        {
            paragraph += line + ""; // 현재 라인을 현재까지 모은 문단에 추가
        }
    }

    // 마지막 문단 처리
    if (!paragraph.empty())
    {
        ofstream out("part" + to_string(++fileCount) + ".txt"); // 분산 저장된 파일 이름
        out << paragraph;
    }

    return fileCount; // 생성된 파일의 수 반환
} 

// 파일을 스레드 개수로 분할하는 함수
int splitFileByThreads(const string &filename, int threadCount)
{
    ifstream file(filename);
    file.seekg(0, ios::end); // 파일 끝으로 이동하여 파일 크기 확인
    int fileSize = file.tellg(); // 파일 크기
    file.seekg(0, ios::beg); // 파일 시작으로 이동

    // 각 파티션의 크기 계산 (파일 크기를 스레드 개수로 나눔)
    int partitionSize = fileSize / threadCount;

    int partNumber = 1; // 파티션 번호 초기화
    string partFileName;
    string line, partition; // 현재 읽은 라인, 현재 파티션 데이터
    int partitionCount = 0; // 현재까지 읽은 파티션의 크기
    while (getline(file, line))
    {
        line += ""; // 개행 문자 추가
        partitionCount += line.size();
        partition += line;

        // 파티션 크기가 계산된 크기를 넘으면 파일에 쓰고 초기화
        if (partitionCount >= partitionSize)
        {
            // 마지막 단어를 다음 파티션으로 넘기기
            size_t lastSpacePos = partition.find_last_of(' ');
            if (lastSpacePos != string::npos)
            {
                string lastWord = partition.substr(lastSpacePos + 1);
                partition.erase(lastSpacePos);

                partFileName = "part" + to_string(partNumber++) + ".txt"; // 파티션 파일 이름
                ofstream partFile(partFileName);
                partFile << partition;
                partFile.close();

                // 다음 파티션의 시작에 마지막 단어 추가
                partition = lastWord;
                partitionCount = lastWord.size();
            }
            else
            {
                partFileName = "part" + to_string(partNumber++) + ".txt"; // 파티션 파일 이름
                ofstream partFile(partFileName);
                partFile << partition;
                partFile.close();
                partition.clear(); // 현재까지 읽은 파티션 초기화
                partitionCount = 0; // 현재까지 읽은 파티션 크기 초기화
            }
        }
    }

    // 마지막 파티션 처리
    if (!partition.empty())
    {
        partFileName = "part" + to_string(partNumber++) + ".txt"; // 파티션 파일 이름
        ofstream partFile(partFileName);
        partFile << partition;
        partFile.close();
    }

    return partNumber - 1; // 생성된 파티션의 수 반환
}


// merge&sort 함수
map<string, vector<int>> mergeSort(int fileCount)
{
    map<string, vector<int>> mergedData; // 합쳐진 데이터를 저장할 맵

    // 각 로컬 결과 파일에서 데이터를 읽어와서 합치기
    for (int i = 1; i <= fileCount; ++i)
    {
        ifstream localFile("localResult" + to_string(i) + ".txt");
        string key;
        int value;

         // 파일에서 키와 값 읽어와서 mergedData에 추가
        while (localFile >> key >> value)
        {
            mergedData[key].push_back(value);
        }
    }

    // 중간 결과를 파일에 저장
    ofstream mergeFile("mergeSort.txt");

    // 합쳐진 데이터를 파일에 쓰기
    for (const auto &pair : mergedData)
    {
        mergeFile << pair.first << ": [";

        // 벡터 내의 값들을 파일에 쓰기
        for (int i = 0; i < pair.second.size(); ++i)
        {
            mergeFile << pair.second[i];
            if (i != pair.second.size() - 1) // 마지막 값이 아니면 쉼표 추가
            {
                mergeFile << ",";
            }
        }
        mergeFile << "]" << endl; // 마지막 값이라면 ]로 마무리
    }

    return mergedData;
}

int main()
{
    auto start_time = chrono::high_resolution_clock::now();

    int option; // 사용자가 파일 분할 방식을 선택
    cout << "파일 분할 방식을 선택해주세요. (1: 문단 수 기준, 2: 스레드(파일 크기) 기준): ";
    cin >> option;

    int fileCount;
    if (option == 1) // 문단 수 기준 분할
    {
        int numParagraphs;
        cout << "파일을 분할할 문단 수 입력: ";
        cin >> numParagraphs;
        fileCount = splitFileByParagraphs("text.txt", numParagraphs);
    }
    else if (option == 2) // 스레드 기준 분할
    {
        int numThreads;
        cout << "파일을 분할할 스레드 수 입력 (4일때 가장 빠름): ";
        cin >> numThreads;
        fileCount = splitFileByThreads("text.txt", numThreads);
    }
    else
    {
        cout << "잘못된 입력입니다." << endl;
        return 0;
    }

    // 각 파일 파트 처리하기 위한 스레드 벡
    vector<thread> threads;
    for (int i = 1; i <= fileCount; ++i)
    {
        // 파일 파트를 처리하는 스레드 생성 및 벡터에 추가
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i));
    }

// 모든 스레드의 종료를 기다림
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
        // reduce 함수를 사용하여 데이터를 합산하고 최종 결과에 추가
        finalResult[it.first] = reducer.reduceValues(it.second);
    }

    auto end_time = chrono::high_resolution_clock::now();

    // 최종 결과 출력
    ofstream resultFile("result.txt");
    for (const auto &pair : finalResult)
    {
        resultFile << pair.first << ": " << pair.second << endl;
    }

// 프로그램 실행 시간 계산 및 출력
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms" << endl;
    return 0;
}