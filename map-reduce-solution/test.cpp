#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

// Key-Value 쌍을 저장하는 구조체
struct Pair {
    string key;
    int value;
};

// mapFunction 함수: 각 단어를 키로 하고, 값을 1로 하는 Pair 객체를 생성
vector<Pair> mapFunction(const string& text) {
    vector<Pair> pairs;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        pairs.push_back({ word, 1 });
    }
    return pairs;
}

// Reduce 함수: 같은 키를 가진 value들을 병합
map<string, int> reduce(const vector<Pair>& pairs) {
    map<string, int> reducedData;
    for (const auto& pair : pairs) {
        reducedData[pair.key] += pair.value;
    }
    return reducedData;
}

int main() {
    // 텍스트 파일 읽기
    ifstream file("text.txt");
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // mapFunction 함수 실행
    vector<Pair> pairs = mapFunction(text);

    // Key 별로 Sorting (Map과 Reduce 간)
    sort(pairs.begin(), pairs.end(), [](const Pair& a, const Pair& b) {
        return a.key < b.key;
        });

    // Reduce 함수 실행
    map<string, int> reducedData = reduce(pairs);

    // 결과 출력
    for (const auto& pair : reducedData) {
        cout << pair.first << ": " << pair.second << "\n";
    }

    return 0;
}
