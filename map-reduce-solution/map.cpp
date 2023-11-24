#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

// Key-Value ���� �����ϴ� ����ü
struct Pair {
    string key;
    int value;
};

// mapFunction �Լ�: �� �ܾ Ű�� �ϰ�, ���� 1�� �ϴ� Pair ��ü�� ����
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
    // �ؽ�Ʈ ���� �б�
    ifstream file("text.txt");
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // mapFunction �Լ� ����
    vector<Pair> pairs = mapFunction(text);

    // ��� ���
    for (const auto& pair : pairs) {
        cout << pair.key << ": " << pair.value << "\n";
    }

    return 0;
}
