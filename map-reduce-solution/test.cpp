#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

// Key-Value ���� �����ϴ� ����ü
struct Pair
{
    string key;
    int value;
};

// mapFunction �Լ�: �� �ܾ Ű�� �ϰ�, ���� 1�� �ϴ� Pair ��ü�� ����
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

// Reduce �Լ�: ���� Ű�� ���� value���� ����
map<string, int> reduce(const vector<Pair> &pairs)
{
    map<string, int> reducedData;
    for (const auto &pair : pairs)
    {
        reducedData[pair.key] += pair.value;
    }
    return reducedData;
}

int main()
{
    // �ؽ�Ʈ ���� �б�
    ifstream file("text.txt");
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // mapFunction �Լ� ����
    auto start_time = chrono::high_resolution_clock::now();
    vector<Pair> pairs = mapFunction(text);

    // Key ���� Sorting (Map�� Reduce ��)
    sort(pairs.begin(), pairs.end(), [](const Pair &a, const Pair &b)
         { return a.key < b.key; });

    // Reduce �Լ� ����
    map<string, int> reducedData = reduce(pairs);
    auto end_time = chrono::high_resolution_clock::now();

    // ��� ���
    for (const auto &pair : reducedData)
    {
        cout << pair.first << ": " << pair.second << "\n";
    }
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    cout << "Execution time: " << duration << " ms\n";
    return 0;
}
