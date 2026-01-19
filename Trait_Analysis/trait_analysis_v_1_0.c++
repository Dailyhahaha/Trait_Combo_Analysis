#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

// 定义城邦羁绊列表及激活所需数量
map<string, int> cityTraitActivate = {
    {"德玛西亚", 3}, {"艾欧尼亚", 3}, {"约德尔人", 2}, {"诺克萨斯", 3},
    {"恕瑞玛", 2}, {"虚空", 2}, {"巨神峰", 1}, {"比尔吉沃特", 3},
    {"皮尔特沃夫", 2}, {"弗雷尔卓德", 3}, {"祖安", 3}, {"暗影岛", 2}
};

// 控制台颜色控制（Windows/Linux通用）
#define RED "\033[31m"    // 红色
#define RESET "\033[0m"   // 重置颜色

// 按空格分割字符串
vector<string> splitBySpace(const string& s) {
    vector<string> res;
    stringstream ss(s);
    string token;
    while (ss >> token) {
        res.push_back(token);
    }
    return res;
}

// 检查是否是城邦羁绊
bool isCityTrait(const string& trait) {
    return cityTraitActivate.find(trait) != cityTraitActivate.end();
}

int main() {
    // 存储弈子-羁绊映射
    map<string, vector<string>> championTraits;

    // 1. 读取弈子羁绊文件
    string filename = "./traits.txt";
    //cout << "请输入弈子羁绊文件路径（例如：traits.txt）：";
    //cin >> filename;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "错误：无法打开文件 " << filename << endl;
        return 1;
    }

    string line;
    int lineNum = 0;
    while (getline(file, line)) {
        lineNum++;
        vector<string> parts = splitBySpace(line);
        if (parts.size() < 2) {
            cerr << "警告：第" << lineNum << "行格式无效，已跳过" << endl;
            continue;
        }
        string champion = parts[0];
        vector<string> traits(parts.begin() + 1, parts.end());
        championTraits[champion] = traits;
    }
    file.close();

    cout << "成功读取 " << championTraits.size() << " 个弈子的羁绊信息" << endl;

    // 2. 获取用户阵容信息
    int population;
    cout << "\n请输入你的阵容人口数：";
    cin >> population;
    if (population <= 0) {
        cerr << "错误：人口数必须大于0" << endl;
        return 1;
    }

    cin.ignore(); // 清除换行符

    string championInput;
    cout << "请输入" << population << "个弈子的名称（空格分隔，一次性输入）：";
    getline(cin, championInput);
    vector<string> inputChampions = splitBySpace(championInput);

    if (inputChampions.size() != population) {
        cerr << "警告：输入的弈子数量(" << inputChampions.size() << ")与人口数(" << population << ")不匹配" << endl;
        return 1;
    }

    // 筛选有效弈子
    vector<string> myChampions;
    for (const string& champ : inputChampions) {
        if (championTraits.find(champ) != championTraits.end()) {
            myChampions.push_back(champ);
        } else {
            cerr << "警告：未找到弈子 [" << champ << "] 的羁绊信息，已跳过" << endl;
        }
    }

    // 3. 统计羁绊数量
    map<string, int> traitCount;
    for (const string& champ : myChampions) {
        for (const string& trait : championTraits[champ]) {
            traitCount[trait]++;
        }
    }

    // 4. 输出统计结果
    cout << "\n=== 你的阵容羁绊统计 ===" << endl;

    // 4.1 输出城邦羁绊（优先激活的，红色显示）
    cout << "\n【城邦羁绊】" << endl;
    // 先输出已激活的城邦羁绊（红色）
    bool hasActivated = false;
    for (const auto& cityPair : cityTraitActivate) {
        string trait = cityPair.first;
        int need = cityPair.second;
        int count = traitCount.count(trait) ? traitCount[trait] : 0;
        
        if (count >= need) { // 已激活
            cout << RED << "[已激活] " << trait << ": " << count << " (需" << need << ")" << RESET << endl;
            hasActivated = true;
            traitCount[trait] = -1; // 标记已输出
        }
    }

    // 再输出未激活但有数量的城邦羁绊
    bool hasUnactivated = false;
    for (const auto& cityPair : cityTraitActivate) {
        string trait = cityPair.first;
        int need = cityPair.second;
        int count = traitCount.count(trait) ? traitCount[trait] : 0;
        
        if (count > 0 && count < need) { // 未激活但有数量
            cout << "[未激活] " << trait << ": " << count << " (需" << need << ")" << endl;
            hasUnactivated = true;
            traitCount[trait] = -1; // 标记已输出
        }
    }

    // 无任何城邦羁绊数量时提示
    if (!hasActivated && !hasUnactivated) {
        cout << "无城邦羁绊" << endl;
    }

    // 4.2 输出其他羁绊
    cout << "\n【其他羁绊】" << endl;
    bool hasOther = false;
    for (const auto& pair : traitCount) {
        if (pair.second > 0) { // 只输出未标记的非城邦羁绊
            cout << pair.first << ": " << pair.second << endl;
            hasOther = true;
        }
    }
    if (!hasOther) {
        cout << "无其他羁绊" << endl;
    }

    return 0;
}