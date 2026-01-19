#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>  // C++17及以上支持

// 基础颜色定义（兼容Windows/Linux/macOS）
#define COLOR_RESET "\033[0m"        // 重置颜色
#define COLOR_WHITE "\033[37m"       // 1费-白色
#define COLOR_GREEN "\033[32m"       // 2费-绿色
#define COLOR_BLUE "\033[34m"        // 3费-蓝色
#define COLOR_PURPLE "\033[35m"      // 4费-紫色
#define COLOR_GOLD "\033[33m"        // 5/7费-金色

// 羁绊等级颜色（保留原有）
#define COLOR_RED "\033[31m"           // 红色
#define COLOR_BROWN "\033[38;5;94m"    // 黄铜色
#define COLOR_SILVER "\033[38;5;248m"  // 银色
#define COLOR_YELLOW "\033[33m"        // 黄金色
#define COLOR_PINK "\033[35m"          // 棱彩色

// 羁绊等级枚举
enum class TraitLevel {
    None,       // 未达成
    RED,        // 红色
    Bronze,     // 黄铜
    Silver,     // 白银
    Gold,       // 黄金
    Prismatic   // 棱彩
};

// 羁绊类型枚举（新增：区分专属独羁绊/普通羁绊/城邦羁绊）
enum class TraitType {
    Exclusive,  // 专属独羁绊（星界游神、解脱者等）
    Normal,     // 普通羁绊（护卫、战士等）
    City        // 城邦羁绊（德玛西亚、艾欧尼亚等，有棱彩）
};

// 羁绊配置结构体
struct TraitConfig {
    std::string name;                  // 羁绊名称
    std::vector<int> requiredCounts;   // 各等级所需弈子数
    bool hasPrismatic;                 // 是否有棱彩级
    TraitType type;                    // 羁绊类型
};

// 弈子结构体（新增费用字段）
struct Champion {
    std::vector<std::string> names;    // 名称+别称
    int cost;                          // 弈子费用（1/2/3/4/5/7）
    std::vector<std::string> traits;   // 所属羁绊
};

// 最终输出的羁绊信息
struct TraitResult {
    std::string name;
    int count;
    TraitLevel level;
    bool isAchieved;
    TraitType type;                    // 羁绊类型
};

// 存储输入的弈子（名称+费用，用于彩色输出）
struct InputChampion {
    std::string inputName;  // 用户输入的名称/别称
    std::string realName;   // 弈子正式名称
    int cost;               // 费用
};

// 预设羁绊配置规则（不变）
std::vector<TraitConfig> initTraitConfigs() {
    std::vector<TraitConfig> configs;
    // 有棱彩的城邦羁绊
    configs.push_back({"德玛西亚", {3, 5, 7, 11}, true, TraitType::City});
    configs.push_back({"艾欧尼亚", {3, 5, 7, 10}, true, TraitType::City});
    configs.push_back({"约德尔人", {2, 4, 6, 8, 10}, true, TraitType::City});
    configs.push_back({"诺克萨斯", {3, 5, 7, 10}, true, TraitType::City});
    configs.push_back({"恕瑞玛", {1, 2, 3, 4}, true, TraitType::City});
    configs.push_back({"比尔吉沃特", {3, 5, 7, 10}, true, TraitType::City});
    // 普通城邦羁绊
    configs.push_back({"弗雷尔卓德", {2, 4, 6}, false, TraitType::City});
    configs.push_back({"皮尔特沃夫", {2, 4, 6}, false, TraitType::City});
    configs.push_back({"祖安", {3, 5, 7}, false, TraitType::City});
    configs.push_back({"暗影岛", {2, 3, 4, 5}, false, TraitType::City});
    configs.push_back({"以绪塔尔", {2, 4, 6}, false, TraitType::City});
    configs.push_back({"巨神峰", {2, 4, 6}, false, TraitType::City});
    // 普通羁绊
    configs.push_back({"护卫", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"战士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"法师", {3, 6, 9}, false, TraitType::Normal});
    configs.push_back({"刺客", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"射手", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"坦克", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"虚空", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"神谕者", {2, 4}, false, TraitType::Normal});
    configs.push_back({"狙神", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"裁决战士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"主宰", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"迅击战士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"斗士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"耀光使", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"神盾使", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"征服者", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"暗裔", {2, 4}, false, TraitType::Normal});
    // 专属独羁绊
    configs.push_back({"星界游神", {1}, false, TraitType::Exclusive});
    configs.push_back({"狂野女猎手", {1}, false, TraitType::Exclusive});
    configs.push_back({"河流之王", {1}, false, TraitType::Exclusive});
    configs.push_back({"腕豪", {1}, false, TraitType::Exclusive});
    configs.push_back({"海克斯机甲", {1}, false, TraitType::Exclusive});
    configs.push_back({"正义巨像", {1}, false, TraitType::Exclusive});
    configs.push_back({"沙漠皇帝", {1}, false, TraitType::Exclusive});
    configs.push_back({"时光守护者", {1}, false, TraitType::Exclusive});
    configs.push_back({"远古恐惧", {1}, false, TraitType::Exclusive});
    configs.push_back({"龙血武姬", {1}, false, TraitType::Exclusive});
    configs.push_back({"山隐之焰", {1}, false, TraitType::Exclusive});
    configs.push_back({"永猎双子", {1}, false, TraitType::Exclusive});
    configs.push_back({"暗裔剑魔", {1}, false, TraitType::Exclusive});
    configs.push_back({"铸星龙王", {1}, false, TraitType::Exclusive});
    configs.push_back({"远古巫灵", {1}, false, TraitType::Exclusive});
    configs.push_back({"系魂圣枪", {1}, false, TraitType::Exclusive});
    configs.push_back({"解脱者", {1}, false, TraitType::Exclusive});
    configs.push_back({"符文法师", {1}, false, TraitType::Exclusive});
    configs.push_back({"不落魔锋", {1}, false, TraitType::Exclusive});
    configs.push_back({"黑暗之女", {1}, false, TraitType::Exclusive});
    configs.push_back({"虚空之女", {1}, false, TraitType::Exclusive});
    return configs;
}

// 从文件读取弈子信息（新增费用解析）
std::vector<Champion> readChampionsFromFile(const std::string& filePath) {
    std::vector<Champion> champions;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "错误：无法打开文件 " << filePath << std::endl;
        return champions;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;

        while (ss >> token) {
            parts.push_back(token);
        }

        // 格式要求：名称/别称 费用 羁绊1 羁绊2...（至少3部分）
        if (parts.size() < 3) {
            std::cerr << "警告：第" << lineNum << "行格式错误（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        Champion champ;
        // 解析名称/别称（第一部分）
        std::string namePart = parts[0];
        std::stringstream nameSS(namePart);
        std::string name;
        while (std::getline(nameSS, name, '/')) {
            name.erase(std::remove_if(name.begin(), name.end(), isspace), name.end());
            if (!name.empty()) {
                champ.names.push_back(name);
            }
        }

        // 解析费用（第二部分，转为数字）
        try {
            champ.cost = std::stoi(parts[1]);
        } catch (...) {
            std::cerr << "警告：第" << lineNum << "行费用格式错误（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        // 解析羁绊（第三部分及以后）
        for (size_t i = 2; i < parts.size(); i++) {
            champ.traits.push_back(parts[i]);
        }

        if (champ.names.empty() || champ.traits.empty()) {
            std::cerr << "警告：第" << lineNum << "行数据无效（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        champions.push_back(champ);
    }

    file.close();
    std::cout << "成功读取 " << champions.size() << " 个弈子信息" << std::endl;
    return champions;
}

// 根据名称/别称查找弈子（返回指针+正式名称）
const Champion* findChampionByAnyName(const std::vector<Champion>& champions, const std::string& inputName, std::string& realName) {
    for (const auto& champ : champions) {
        for (size_t i = 0; i < champ.names.size(); i++) {
            if (champ.names[i] == inputName) {
                realName = champ.names[0];  // 正式名称取第一个
                return &champ;
            }
        }
    }
    realName = "";
    return nullptr;
}

// 获取费用对应的颜色
std::string getCostColor(int cost) {
    switch (cost) {
        case 1: return COLOR_WHITE;
        case 2: return COLOR_GREEN;
        case 3: return COLOR_BLUE;
        case 4: return COLOR_PURPLE;
        case 5: 
        case 7: return COLOR_GOLD;
        default: return COLOR_RESET;  // 未知费用默认白色
    }
}

// 获取羁绊结果的显示颜色（新增：专属羁绊用红色）
std::string getResultColor(const TraitResult& result) {
    // 专属独羁绊优先用红色
    if (result.type == TraitType::Exclusive && result.isAchieved) {
        return COLOR_RED;
    }
    // 普通羁绊按等级取色
    switch (result.level) {
        case TraitLevel::Bronze: return COLOR_BROWN;
        case TraitLevel::Silver: return COLOR_SILVER;
        case TraitLevel::Gold: return COLOR_YELLOW;
        case TraitLevel::Prismatic: return COLOR_PINK;
        default: return COLOR_RESET;
    }
}
// 原有工具函数（不变）
TraitConfig getTraitConfig(const std::vector<TraitConfig>& configs, const std::string& traitName) {
    for (const auto& config : configs) {
        if (config.name == traitName) {
            return config;
        }
    }
    return {traitName, {}, false};
}

TraitLevel judgeTraitLevel(const TraitConfig& config, int count) {
    if (config.requiredCounts.empty() || count < config.requiredCounts[0]) {
        return TraitLevel::None;
    }

    if (config.hasPrismatic) {
        if (count >= config.requiredCounts.back()) {
            return TraitLevel::Prismatic;
        } else if (count >= config.requiredCounts[2]) {
            return TraitLevel::Gold;
        } else if (count >= config.requiredCounts[1]) {
            return TraitLevel::Silver;
        } else if (count >= config.requiredCounts[0]) {
            return TraitLevel::Bronze;
        }
    } else {
        if (count >= config.requiredCounts.back()) {
            return TraitLevel::Gold;
        } else if (config.requiredCounts.size() >= 2 && count >= config.requiredCounts[1]) {
            return TraitLevel::Silver;
        } else if (count >= config.requiredCounts[0]) {
            return TraitLevel::Bronze;
        }
    }
    return TraitLevel::None;
}

bool compareTraitResult(const TraitResult& a, const TraitResult& b) {
    if (a.isAchieved != b.isAchieved) {
        return a.isAchieved > b.isAchieved;
    }
    if (a.level != b.level) {
        return static_cast<int>(a.level) > static_cast<int>(b.level);
    }
    return a.name < b.name;
}

std::string getLevelColor(TraitLevel level) {
    switch (level) {
        case TraitLevel::Bronze: return COLOR_BROWN;
        case TraitLevel::Silver: return COLOR_SILVER;
        case TraitLevel::Gold: return COLOR_YELLOW;
        case TraitLevel::Prismatic: return COLOR_PINK;
        default: return COLOR_RESET;
    }
}

std::string getLevelText(TraitLevel level) {
    switch (level) {
        case TraitLevel::Bronze: return "黄铜级";
        case TraitLevel::Silver: return "白银级";
        case TraitLevel::Gold: return "黄金级";
        case TraitLevel::Prismatic: return "棱彩级";
        default: return "未达成";
    }
}

int main() {
    // 1. 初始化配置和读取弈子
    std::vector<TraitConfig> allTraitConfigs = initTraitConfigs();
    std::string filePath = "../Trait_information/tft_champions_trait.txt";  // 替换为你的文件路径
    std::vector<Champion> allChampions = readChampionsFromFile(filePath);
    if (allChampions.empty()) {
        std::cerr << "错误：未读取到任何弈子信息，程序退出" << std::endl;
        return 1;
    }

    // 2. 输入人口数
    int population;
    std::cout << "\n请输入阵容人口数：";
    std::cin >> population;
    std::cin.ignore();

    // 3. 输入弈子名称
    std::string championInput;
    std::cout << "请输入阵容弈子名称（空格分隔，支持别称）：";
    std::getline(std::cin, championInput);

    // 4. 解析输入并统计（新增存储输入弈子的费用）
    std::vector<InputChampion> inputChamps;  // 存储输入的弈子（用于彩色输出）
    std::map<std::string, int> traitCountMap;
    std::stringstream ss(championInput);
    std::string name;

    while (ss >> name) {
        std::string realName;
        const Champion* champ = findChampionByAnyName(allChampions, name, realName);
        if (champ != nullptr) {
            // 统计羁绊
            for (const auto& trait : champ->traits) {
                traitCountMap[trait]++;
            }
            // 记录输入的弈子（用于彩色输出）
            inputChamps.push_back({name, realName, champ->cost});
            // 输出识别结果
            std::cout << "已识别弈子：" << name << "（正式名：" << realName << "，费用：" << champ->cost << "费）→ 羁绊：";
            for (size_t i = 0; i < champ->traits.size(); i++) {
                if (i > 0) std::cout << "、";
                std::cout << champ->traits[i];
            }
            std::cout << std::endl;
        } else {
            std::cerr << "警告：未找到弈子[" << name << "]的信息，已忽略！" << std::endl;
        }
    }

    // 5. 彩色输出阵容弈子（核心新增）
    std::cout << "\n===== 阵容弈子列表 =====" << std::endl;
    std::cout << "当前阵容弈子：";
    for (size_t i = 0; i < inputChamps.size(); i++) {
        if (i > 0) std::cout << " ";
        // 按费用输出对应颜色的弈子名称
        std::string color = getCostColor(inputChamps[i].cost);
        std::cout << color << inputChamps[i].inputName << COLOR_RESET;
    }
    std::cout << std::endl;

    // 6. 生成羁绊结果（原有逻辑不变）
    std::vector<TraitResult> traitResults;
    for (const auto& pair : traitCountMap) {
        TraitConfig config = getTraitConfig(allTraitConfigs, pair.first);
        TraitLevel level = judgeTraitLevel(config, pair.second);
        traitResults.push_back({pair.first, pair.second, level, level != TraitLevel::None});
    }
    // for (const auto& config : allTraitConfigs) {
    //     if (traitCountMap.find(config.name) == traitCountMap.end()) {
    //         traitResults.push_back({config.name, 0, TraitLevel::None, false});
    //     }
    // }

    // 7. 排序并输出羁绊结果
    std::sort(traitResults.begin(), traitResults.end(), compareTraitResult);
    std::cout << "\n===== 阵容羁绊分析结果 =====" << std::endl;
    for (const auto& result : traitResults) {
        std::string color = getResultColor(result);
        std::string levelText = getLevelText(result.level);
        std::cout << color 
                  << "【" << result.name << "】：" << result.count 
                  << COLOR_RESET << std::endl;
    }

    return 0;
}