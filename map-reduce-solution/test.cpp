#include <iostream> // 표준 입출력을 위한 라이브러리
#include <fstream> // 파일 입출력을 위한 라이브러리
#include <string> // 문자열을 사용하기 위한 라이브러리
#include <vector> // 벡터를 사용하기 위한 라이브러리
#include <map> // 맵을 사용하기 위한 라이브러리
#include <sstream> // 문자열 스트림을 사용하기 위한 라이브러리
#include <algorithm> // 알고리즘을 사용하기 위한 라이브러리 (예: sort 등)
#include <thread> // 다중 스레딩을 위한 라이브러리
#include <mutex> // 동기화를 위한 라이브러리

using namespace std; // 표준 라이브러리의 네임스페이스를 사용

struct Pair { // "Pair"라는 이름의 구조체 정의
    string key; // 키를 저장하는 문자열
    int value; // 값 저장하는 정수
};

class MapFunction { // "MapFunction"이라는 클래스 정의
public:
    vector<Pair> operator()(const string& text) const { // 함수 호출 연산자를 오버로딩하여 맵 함수 정의
        vector<Pair> pairs; // Pair 객체를 저장하는 벡터
        stringstream ss(text); // 문자열 스트림 생성
        string word; // 단어를 저장할 문자열
        while (ss >> word) { // 스트림에서 단어를 가져옴
            pairs.push_back({ word, 1 }); // 단어와 1로 구성된 Pair 객체를 벡터에 추가
        }
        return pairs; // 벡터 반환
    }
};

class ReduceFunction { // "ReduceFunction"이라는 클래스 정의
public:
    map<string, int> operator()(const vector<Pair>& pairs) const { // 함수 호출 연산자를 오버로딩하여 리듀스 함수 정의
        map<string, int> reducedData; // 리듀스된 데이터를 저장하는 맵
        for (const auto& pair : pairs) { // 벡터에 있는 모든 Pair 객체에 대해
            reducedData[pair.key] += pair.value; // 맵에 단어와 값을 추가 (동일 단어 시 값 증가)
        }
        return reducedData; // 맵 반환
    }
};

void processFilePart(const string& filename, int partNumber, MapFunction mapFunction) { // 파일의 일부를 처리하는 함수
    ifstream file(filename); // 입력 파일 스트림 생성
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()); // 파일의 내용을 문자열로 읽어옴
    vector<Pair> pairs = mapFunction(text); // 맵 함수를 사용하여 Pair 객체 생성

    ofstream outputFile("localResult" + to_string(partNumber) + ".txt"); // 출력 파일 스트림 생성
    for (const auto& pair : pairs) { // 모든 Pair 객체에 대해
        outputFile << pair.key << " " << pair.value << '\n'; // 파일에 단어와 값 쓰기
    }
}

int splitFile(const string& filename) { // 파일을 여러 부분으로 분할하는 함수
    ifstream file(filename); // 입력 파일 스트림 생성
    string line, paragraph; // 줄과 단락을 저장하는 문자열
    int fileCount = 0; // 파일 수
    int paragraphCount = 0; // 단락 수
    while (getline(file, line)) { // 파일의 각 줄에 대해
        if (line.empty()) { // 줄이 비어 있으면
            paragraphCount++; // 단락 수 증가
            if (paragraphCount == 1540) { // 단락 수가 1540이면
                ofstream out("part" + to_string(++fileCount) + ".txt"); // 새 파일 스트림 생성
                out << paragraph; // 단락을 파일에 쓰기
                paragraph.clear(); // 단락 초기화
                paragraphCount = 0; // 단락 수 초기화
            }
        }
        else {
            paragraph += line + "\n"; // 줄을 단락에 추가
        }
    }

    if (!paragraph.empty()) { // 마지막 단락이 비어 있지 않으면
        ofstream out("part" + to_string(++fileCount) + ".txt"); // 새 파일 스트림 생성
        out << paragraph; // 단락을 파일에 쓰기
    }

    return fileCount; // 파일 수 반환
}

int main() { // 메인 함수
    auto start_time = chrono::high_resolution_clock::now(); // 시작 시간 측정
    int fileCount = splitFile("text.txt"); // 파일 분할

    vector<thread> threads; // 스레드를 저장하는 벡터
    MapFunction mapFunction; // 맵 함수 객체 생성

    for (int i = 1; i <= fileCount; ++i) { // 각 파일에 대해
        threads.push_back(thread(processFilePart, "part" + to_string(i) + ".txt", i, mapFunction)); // 스레드를 생성하여 벡터에 추가
    }

    for (auto& t : threads) { // 모든 스레드에 대해
        t.join(); // 스레드가 끝날 때까지 대기
    }

    ReduceFunction reduceFunction; // 리듀스 함수 객체 생성
    map<string, int> finalResult; // 최종 결과를 저장하는 맵
    for (int i = 1; i <= fileCount; ++i) { // 각 로컬 결과 파일에 대해
        ifstream localFile("localResult" + to_string(i) + ".txt"); // 입력 파일 스트림 생성
        string key; // 키를 저장하는 문자열
        int value; // 값을 저장하는 정수
        vector<Pair> pairs; // Pair 객체를 저장하는 벡터
        while (localFile >> key >> value) { // 파일의 각 줄에 대해
            pairs.push_back({ key, value }); // 벡터에 Pair 객체 추가
        }
        map<string, int> reducedData = reduceFunction(pairs); // 리듀스 함수를 사용하여 데이터 축소
        for (const auto& pair : reducedData) { // 축소된 데이터의 각 항목에 대해
            finalResult[pair.first] += pair.second; // 최종 결과에 항목 추가 (동일 단어 시 값 증가)
        }
    }
    auto end_time = chrono::high_resolution_clock::now(); // 종료 시간 측정

    for (const auto& pair : finalResult) { // 최종 결과의 각 항목에 대해
        cout << pair.first << ": " << pair.second << '\n'; // 단어와 값을 출력
    }

    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count(); // 실행 시간 계산
    cout << "Execution time: " << duration << " ms\n"; // 실행 시간 출력

    return 0; // 프로그램 종료
}
