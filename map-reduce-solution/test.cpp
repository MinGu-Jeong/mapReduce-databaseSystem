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

// key와 value를 가지는 구조체
struct Pair
{
    string key;
    int value;
};

// Map 클래스
class Map
{
public:
    // 주어진 문자열을 토큰화하여 Pair 벡터를 반환하는 함수
    vector<Pair> mapFunction(const string &text)
    {
        vector<Pair> pairs; // Pair들을 저장할 벡터
        stringstream ss(text); // 입력 문자열로부터 stringstream 생성
        string word;

        // 문자열을 단어 단위로 읽어들여 Pair를 생성하고 벡터에 추가
        while (ss >> word)
        {
            pairs.push_back({word, 1});
        }

        return pairs; // 벡터 반환
    }
};

// Reduce 클래스
class Reduce
{
public:
    // 정수 벡터의 값을 합하여 반환하는 함수
    int reduceValues(const vector<int> &values)
    {
        int sum = 0; // 합을 저장할 변수
        for (const auto &value : values)
        {
            sum += value; // 합 계산
        }
        return sum; // 최종 합 반환
    }
};

// 파일 일부를 처리하고 결과를 파일에 저장하는 함수
void processFilePart(const string &filename, int partNumber)
{
    ifstream file(filename);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); // 파일 내용을 문자열로 읽음

    Map mapper;
    vector<Pair> pairs = mapper.mapFunction(text); // Map 클래스의 mapFunction을 사용하여 단어를 Pair 벡터로 얻음

    ofstream outputFile("localResult" + to_string(partNumber) + ".txt"); // 출력 파일에 쓰기
    for (const auto &pair : pairs)
    {
        outputFile << pair.key << " " << pair.value << '\n';
    }
}

// 파일을 문단 수로 분할하는 함수
int splitFileByParagraphs(const string& filename, int numParagraphs)
{
    ifstream file(filename); // 파일 열기
    string line, paragraph;
    int fileCount = 0; // 파일 개수
    int paragraphCount = 0; // 문단 수
    while (getline(file, line))
    {
        if (line.empty()) // 문단이 끝났다고 가정
        { 
            paragraphCount++;
            if (paragraphCount == numParagraphs) // 사용자 입력 문단마다 파일을 분할
            {
                ofstream out("part" + to_string(++fileCount) + ".txt"); // 출력 파일에 문단 쓰기
                out << paragraph;
                paragraph.clear(); // 문단 초기화
                paragraphCount = 0;
            }
        }
        else
        {
            paragraph += line + ""; // 문단 저장
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

// 파일을 스레드 개수로 분할하는 함수
int splitFileByThreads(const string& filename, int threadCount)
{
    ifstream file(filename); // 파일 열기
    file.seekg(0, ios::end); // 파일 커서를 맨 끝으로 이동
    int fileSize = file.tellg(); // 파일 크기 저장
    file.seekg(0, ios::beg); // 파일 커서를 앞으로 이동

    // 각 파티션의 크기 계산 (파일 크기를 스레드 개수로 나눔)
    int partitionSize = fileSize / threadCount;

    int partNumber = 1;
    string partFileName;
    string line, partition;
    int partitionCount = 0;
    while (getline(file, line))
    {
        line += "";  // 개행 문자 추가
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

                partFileName = "part" + to_string(partNumber++) + ".txt";
                ofstream partFile(partFileName);
                partFile << partition;
                partFile.close();

                // 다음 파티션의 시작에 마지막 단어 추가
                partition = lastWord;
                partitionCount = lastWord.size();
            }
            else
            {
                partFileName = "part" + to_string(partNumber++) + ".txt";
                ofstream partFile(partFileName);
                partFile << partition;
                partFile.close();
                partition.clear();
                partitionCount = 0;
            }
        }
    }

    // 마지막 파티션 처리
    if (!partition.empty())
    {
        partFileName = "part" + to_string(partNumber++) + ".txt";
        ofstream partFile(partFileName);
        partFile << partition;
        partFile.close();
    }

    return partNumber - 1; // 생성된 파티션(파일)의 개수 반환
}


// 여러 개의 로컬 결과 파일로부터 데이터를 읽어와 병합하고 결과를 반환하는 함수
map<string, vector<int>> mergeSort(int fileCount)
{
    map<string, vector<int>> mergedData;
    
    // 각 로컬 결과 파일에 대해 반복
    for (int i = 1; i <= fileCount; ++i)
    {
        ifstream localFile("localResult" + to_string(i) + ".txt");
        string key;
        int value;

        // 파일에서 키와 값을 읽어와서 mergedData에 추가
        while (localFile >> key >> value)
        {
            mergedData[key].push_back(value);
        }
    }

    // 중간 결과를 파일에 저장
    ofstream mergeFile("mergeSort.txt");
    for (const auto &pair : mergedData)
    {
        mergeFile << pair.first << ": ["; // // 키 출력
        // 벡터의 각 원소를 출력
        for (int i = 0; i < pair.second.size(); ++i)
        {
            mergeFile << pair.second[i];
            // 마지막 원소가 아니라면 쉼표 출력
            if (i != pair.second.size() - 1)
            {
                mergeFile << ",";
            }
        }
        mergeFile << "]" << endl; // 벡터 출력 완료 후 개행
    }

    return mergedData; // 병합된 데이터 반환
}

int main()
{
    int option; // 파일 분할 방식 선택
    cout << "파일 분할 방식을 선택해주세요. (1: 문단 수 기준, 2: 스레드(파일 크기) 기준): ";
    cin >> option;

    int fileCount;
    if (option == 1)
    {
        int numParagraphs;
        cout << "파일을 분할할 문단 수 입력: ";
        cin >> numParagraphs;
        fileCount = splitFileByParagraphs("text.txt", numParagraphs);
    }
    else if (option == 2)
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
    
    auto start_time = chrono::high_resolution_clock::now(); // 시간 측정 시작

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
    auto end_time = chrono::high_resolution_clock::now();
    // 모든 로컬 결과 파일을 병합
    map<string, vector<int>> mergedData = mergeSort(fileCount);

    // 병합된 데이터를 reduce 처리
    Reduce reducer;
    map<string, int> finalResult;
    for (const auto &it : mergedData)
    {
        finalResult[it.first] = reducer.reduceValues(it.second);
    }

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