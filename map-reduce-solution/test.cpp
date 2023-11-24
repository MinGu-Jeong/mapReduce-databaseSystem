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

mutex mtx; // For thread-safe access to the global data structure

// Key-Value ���� �����ϴ� ����ü
struct Pair {
    string key;
    int value;
};

// mapFunction �Լ�: �� �ܾ Ű�� �ϰ�, ���� 1�� �ϴ� Pair ��ü�� ����
void mapFunction(const string& text, vector<Pair>& pairs) {
    stringstream ss(text);
    string word;
    while (ss >> word) {
        mtx.lock(); // Ensure thread-safe access to the shared data
        pairs.push_back({ word, 1 });
        mtx.unlock(); // Release the lock
    }
}

// Reduce �Լ�: ���� Ű�� ���� value���� ����
map<string, int> reduce(const vector<Pair>& pairs) {
    map<string, int> reducedData;
    for (const auto& pair : pairs) {
        reducedData[pair.key] += pair.value;
    }
    return reducedData;
}

int main() {
    // �ؽ�Ʈ ���� �б�
    ifstream file("text.txt");
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // Create a vector to store the key-value pairs
    vector<Pair> pairs;

    // Create multiple threads and run the map function on each thread
    vector<thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.push_back(thread(mapFunction, text, ref(pairs)));
    }

    // Wait for all threads to complete
    for (auto& th : threads) {
        th.join();
    }

    // Key ���� Sorting (Map�� Reduce ��)
    sort(pairs.begin(), pairs.end(), [](const Pair& a, const Pair& b) {
        return a.key < b.key;
    });

    // Reduce �Լ� ����
    map<string, int> reducedData = reduce(pairs);

    // ��� ���
    for (const auto& pair : reducedData) {
        cout << pair.first << ": " << pair.second << endl;
    }

    return 0;
}
