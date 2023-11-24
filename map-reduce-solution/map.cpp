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

int main() {
    // 텍스트 파일 읽기
    ifstream file("text.txt");
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // mapFunction 함수 실행
    vector<Pair> pairs = mapFunction(text);

    // 결과 출력
    for (const auto& pair : pairs) {
        cout << pair.key << ": " << pair.value << "\n";
    }

    return 0;
}
