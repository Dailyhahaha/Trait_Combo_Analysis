#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

using namespace std;

// 定义城邦羁绊列表及激活所需数量
map<string, int> cityTraitActivate = {
    {"德玛西亚", 3}, {"艾欧尼亚", 3}, {"约德尔人", 2}, {"诺克萨斯", 3}, {"恕瑞玛", 2}, {"虚空", 2}, {"巨神峰", 1}, {"比尔吉沃特", 3}, {"皮尔特沃夫", 2}, {"弗雷尔卓德", 3}, {"祖安", 3}, {"暗影岛", 2}};

// 控制台颜色控制（Windows/Linux通用）
#define RED "\033[31m"  // 红色
#define RESET "\033[0m" // 重置颜色

// 去除字符串首尾空格
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

// 按指定分隔符分割字符串（处理空值）
vector<string> split(const string &s, char delimiter)
{
    vector<string> res;
    stringstream ss(s);
    string token;
    while (getline(ss, token, delimiter))
    {
        string trimmedToken = trim(token);
        if (!trimmedToken.empty())
        { // 过滤空的分割结果
            res.push_back(trimmedToken);
        }
    }
    return res;
}

// 按空格分割字符串（兼容多个空格）
vector<string> splitBySpace(const string &s)
{
    vector<string> res;
    stringstream ss(s);
    string token;
    while (ss >> token)
    {
        res.push_back(token);
    }
    return res;
}

// 检查是否是城邦羁绊
bool isCityTrait(const string &trait)
{
    return cityTraitActivate.find(trait) != cityTraitActivate.end();
}

int main()
{
    while (1)
    {
        // 存储：主名称 -> 羁绊列表
        map<string, vector<string>> championTraits;
        // 存储：别名/主名称 -> 主名称（核心：实现别名匹配）
        map<string, string> aliasToChampion;

        // 1. 读取带别名的弈子羁绊文件
        string filename = "../Trait_information/tft_champions_trait.txt";
        // cout << "请输入弈子羁绊文件路径（例如：traits.txt）：";
        // cin >> filename;

        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "错误：无法打开文件 " << filename << endl;
            return 1;
        }

        string line;
        int lineNum = 0;
        while (getline(file, line))
        {
            lineNum++;
            vector<string> parts = splitBySpace(line);
            if (parts.size() < 1)
            { // 至少要有弈子名称/别名部分
                cerr << "警告：第" << lineNum << "行格式无效，已跳过" << endl;
                continue;
            }

            // 拆分名称/别名部分（格式：主名称/别名1/别名2）
            string nameAliasPart = parts[0];
            vector<string> nameAliases = split(nameAliasPart, '/');
            if (nameAliases.empty())
            {
                cerr << "警告：第" << lineNum << "行无有效弈子名称，已跳过" << endl;
                continue;
            }

            // 主名称（第一个元素）
            string mainChampion = nameAliases[0];
            // 提取羁绊列表（空格后的所有部分）
            vector<string> traits(parts.begin() + 1, parts.end());
            // 存储主名称对应的羁绊
            championTraits[mainChampion] = traits;

            // 建立别名映射：主名称和所有别名都指向主名称
            for (const string &alias : nameAliases)
            {
                aliasToChampion[alias] = mainChampion;
            }
        }
        file.close();

        cout << "成功读取 " << championTraits.size() << " 个弈子的羁绊信息（含别名匹配）" << endl;

        // 2. 获取用户阵容信息
        int population;
        cout << "\n请输入你的阵容人口数：";
        cin >> population;
        if (population <= 0)
        {
            cerr << "错误：人口数必须大于0" << endl;
            return 1;
        }

        cin.ignore(); // 清除换行符

        string championInput;
        cout << "请输入" << population << "个弈子的名称/别名（空格分隔，一次性输入）：";
        getline(cin, championInput);
        vector<string> inputChampions = splitBySpace(championInput);

        if (inputChampions.size() != population)
        {
            cerr << "警告：输入的弈子数量(" << inputChampions.size() << ")与人口数(" << population << ")不匹配" << endl;
            return 1;
        }

        // 3. 匹配用户输入（支持别名），筛选有效弈子
        vector<string> myChampions;
        for (const string &inputName : inputChampions)
        {
            // 查找输入的名称（别名/主名称）对应的主名称
            auto it = aliasToChampion.find(inputName);
            if (it != aliasToChampion.end())
            {
                string mainName = it->second;
                myChampions.push_back(mainName);
                //cout << "提示：已匹配别名 [" << inputName << "] -> 主名称 [" << mainName << "]" << endl;
            }
            else
            {
                //cerr << "警告：未找到弈子/别名 [" << inputName << "] 的信息，已跳过" << endl;
            }
        }

        // 4. 统计羁绊数量
        map<string, int> traitCount;
        for (const string &mainName : myChampions)
        {
            for (const string &trait : championTraits[mainName])
            {
                traitCount[trait]++;
            }
        }

        // 5. 输出统计结果
        cout << "\n=== 你的阵容羁绊统计 ===" << endl;

        // 5.1 输出城邦羁绊（优先激活的，红色显示）
        cout << "\n【城邦羁绊】" << endl;
        bool hasActivated = false;
        for (const auto &cityPair : cityTraitActivate)
        {
            string trait = cityPair.first;
            int need = cityPair.second;
            int count = traitCount.count(trait) ? traitCount[trait] : 0;

            if (count >= need)
            { // 已激活
                cout << RED << "[已激活] " << trait << ": " << count << " (需" << need << ")" << RESET << endl;
                hasActivated = true;
                traitCount[trait] = -1; // 标记已输出
            }
        }

        // 输出未激活但有数量的城邦羁绊
        bool hasUnactivated = false;
        for (const auto &cityPair : cityTraitActivate)
        {
            string trait = cityPair.first;
            int need = cityPair.second;
            int count = traitCount.count(trait) ? traitCount[trait] : 0;

            if (count > 0 && count < need)
            { // 未激活但有数量
                cout << "[未激活] " << trait << ": " << count << " (需" << need << ")" << endl;
                hasUnactivated = true;
                traitCount[trait] = -1; // 标记已输出
            }
        }

        // 无任何城邦羁绊数量时提示
        if (!hasActivated && !hasUnactivated)
        {
            cout << "无城邦羁绊" << endl;
        }

        // 5.2 输出其他羁绊
        cout << "\n【其他羁绊】" << endl;
        bool hasOther = false;
        for (const auto &pair : traitCount)
        {
            if (pair.second > 0)
            { // 只输出未标记的非城邦羁绊
                cout << pair.first << ": " << pair.second << endl;
                hasOther = true;
            }
        }
        if (!hasOther)
        {
            cout << "无其他羁绊" << endl;
        }
    }
    return 0;
}