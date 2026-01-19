#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>  // C++17及以上支持，用于判断文件是否存在

// 颜色定义（Windows终端需开启ANSI支持，Linux/macOS直接生效）
#define COLOR_BROWN "\033[38;5;94m"    // 黄铜色
#define COLOR_SILVER "\033[38;5;248m"  // 银色
#define COLOR_YELLOW "\033[33m"        // 黄金色
#define COLOR_PINK "\033[35m"          // 棱彩色
#define COLOR_RED "\033[31m"           // 红色
#define COLOR_RESET "\033[0m"          // 重置颜色

// 羁绊等级枚举
enum class TraitLevel {
    None,       // 未达成
    RED,        // 红色
    Bronze,     // 黄铜
    Silver,     // 白银
    Gold,       // 黄金
    Prismatic   // 棱彩
};

// 羁绊配置结构体：存储羁绊等级对应的弈子数、是否有棱彩
struct TraitConfig {
    std::string name;                  // 羁绊名称
    std::vector<int> requiredCounts;   // 各等级所需弈子数（按红色/黄铜/白银/黄金/棱彩顺序）
    bool hasPrismatic;                 // 是否有棱彩级
};

// 弈子结构体（新增别称列表）
struct Champion {
    std::vector<std::string> names;    // 名称+所有别称（如["基兰", "时光", "时光老头"]）
    std::vector<std::string> traits;   // 所属羁绊
};

// 最终输出的羁绊信息
struct TraitResult {
    std::string name;
    int count;                 // 阵容中该羁绊的弈子数
    TraitLevel level;          // 羁绊等级
    bool isAchieved;           // 是否达成（等级≥Bronze）
};

// 预设羁绊配置规则（按需求调整）
std::vector<TraitConfig> initTraitConfigs() {
    std::vector<TraitConfig> configs;
    // 有棱彩的城邦羁绊（最后一级为棱彩）
    configs.push_back({"德玛西亚", {3, 5, 7, 11}, true});
    configs.push_back({"艾欧尼亚", {2, 4, 6, 8}, true});
    configs.push_back({"约德尔人", {2, 4, 6, 10}, true});
    configs.push_back({"诺克萨斯", {3, 6, 9, 12}, true});
    configs.push_back({"恕瑞玛", {2, 4, 6, 9}, true});
    // 普通羁绊（无棱彩，黄金为最高级）
    configs.push_back({"护卫", {2, 4, 6}, false});
    configs.push_back({"战士", {2, 4, 6}, false});
    configs.push_back({"法师", {3, 6, 9}, false});
    configs.push_back({"刺客", {2, 4, 6}, false});
    configs.push_back({"射手", {2, 4, 6}, false});
    configs.push_back({"坦克", {2, 4, 6}, false});
    configs.push_back({"虚空", {2, 4, 6}, false});
    configs.push_back({"比尔吉沃特", {2, 4, 6}, false});
    configs.push_back({"弗雷尔卓德", {2, 4, 6}, false});
    configs.push_back({"皮尔特沃夫", {2, 4, 6}, false});
    configs.push_back({"祖安", {2, 4, 6}, false});
    configs.push_back({"暗影岛", {2, 4, 6}, false});
    configs.push_back({"以绪塔尔", {2, 4, 6}, false});
    configs.push_back({"巨神峰", {2, 4, 6}, false});
    configs.push_back({"星界游神", {1}, false});  // 独有的羁绊（1个即可达成）
    configs.push_back({"神谕者", {2, 4}, false});
    configs.push_back({"狙神", {2, 4, 6}, false});
    configs.push_back({"裁决战士", {2, 4, 6}, false});
    configs.push_back({"主宰", {2, 4, 6}, false});
    configs.push_back({"迅击战士", {2, 4, 6}, false});
    configs.push_back({"斗士", {2, 4, 6}, false});
    configs.push_back({"耀光使", {2, 4, 6}, false});
    configs.push_back({"神盾使", {2, 4, 6}, false});
    configs.push_back({"征服者", {2, 4, 6}, false});
    configs.push_back({"狂野女猎手", {1}, false});
    configs.push_back({"河流之王", {1}, false});
    configs.push_back({"腕豪", {1}, false});
    configs.push_back({"海克斯机甲", {1}, false});
    configs.push_back({"暗裔", {2, 4}, false});
    configs.push_back({"正义巨像", {1}, false});
    configs.push_back({"沙漠皇帝", {1}, false});
    configs.push_back({"时光守护者", {1}, false});
    configs.push_back({"远古恐惧", {1}, false});
    configs.push_back({"龙血武姬", {1}, false});
    configs.push_back({"山隐之焰", {1}, false});
    configs.push_back({"永猎双子", {1}, false});
    configs.push_back({"暗裔剑魔", {1}, false});
    configs.push_back({"铸星龙王", {1}, false});
    configs.push_back({"远古巫灵", {1}, false});
    configs.push_back({"系魂圣枪", {1}, false});
    configs.push_back({"解脱者", {1}, false});
    configs.push_back({"符文法师", {1}, false});
    configs.push_back({"不落魔锋", {1}, false});
    configs.push_back({"黑暗之女", {1}, false});
    configs.push_back({"虚空之女", {1}, false});
    return configs;
}

// 从文件读取弈子信息（解析别称和羁绊）
std::vector<Champion> readChampionsFromFile(const std::string& filePath) {
    std::vector<Champion> champions;
    std::ifstream file(filePath);

    // 检查文件是否存在
    if (!file.is_open()) {
        std::cerr << "错误：无法打开文件 " << filePath << "（请检查文件路径是否正确）" << std::endl;
        return champions;
    }

    std::string line;
    int lineNum = 0;
    // 逐行解析
    while (std::getline(file, line)) {
        lineNum++;
        // 跳过空行
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;

        // 按空格分割行内容
        while (ss >> token) {
            parts.push_back(token);
        }

        // 格式校验：至少包含「名称/别称」+「1个羁绊」
        if (parts.size() < 2) {
            std::cerr << "警告：第" << lineNum << "行格式错误（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        Champion champ;
        // 解析第一部分（名称+别称，按/分割）
        std::string namePart = parts[0];
        std::stringstream nameSS(namePart);
        std::string name;
        while (std::getline(nameSS, name, '/')) {
            // 去除名称中的空格（如果有）
            name.erase(std::remove_if(name.begin(), name.end(), isspace), name.end());
            if (!name.empty()) {
                champ.names.push_back(name);
            }
        }

        // 解析后续部分（羁绊）
        for (size_t i = 1; i < parts.size(); i++) {
            champ.traits.push_back(parts[i]);
        }

        // 校验：必须有名称和至少1个羁绊
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

// 根据输入的名称（或别称）查找弈子
const Champion* findChampionByAnyName(const std::vector<Champion>& champions, const std::string& inputName) {
    for (const auto& champ : champions) {
        for (const auto& name : champ.names) {
            if (name == inputName) {
                return &champ;
            }
        }
    }
    return nullptr;
}

// 根据羁绊名称获取配置
TraitConfig getTraitConfig(const std::vector<TraitConfig>& configs, const std::string& traitName) {
    for (const auto& config : configs) {
        if (config.name == traitName) {
            return config;
        }
    }
    // 未找到的羁绊（默认无等级，可手动添加到initTraitConfigs）
    return {traitName, {}, false};
}

// 判定羁绊等级
TraitLevel judgeTraitLevel(const TraitConfig& config, int count) {
    if (config.requiredCounts.empty() || count < config.requiredCounts[0]) {
        return TraitLevel::None;
    }

    // 有棱彩的羁绊（最后一级为棱彩）
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
    }
    // 无棱彩的羁绊（黄金为最高级）
    else {
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

// 排序规则：达成优先 → 等级从高到低 → 名称字母序
bool compareTraitResult(const TraitResult& a, const TraitResult& b) {
    if (a.isAchieved != b.isAchieved) {
        return a.isAchieved > b.isAchieved;  // 达成的排前面
    }
    // 等级从高到低（棱彩>黄金>白银>黄铜>未达成）
    if (a.level != b.level) {
        return static_cast<int>(a.level) > static_cast<int>(b.level);
    }
    // 等级相同按名称拼音序排序（中文按ASCII码排序，效果近似拼音序）
    return a.name < b.name;
}

// 获取等级对应的颜色字符串
std::string getLevelColor(TraitLevel level) {
    switch (level) {
        case TraitLevel::Bronze: return COLOR_BROWN;
        case TraitLevel::Silver: return COLOR_SILVER;
        case TraitLevel::Gold: return COLOR_YELLOW;
        case TraitLevel::Prismatic: return COLOR_PINK;
        default: return COLOR_RESET;
    }
}

// 获取等级对应的文字描述
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
    // 1. 初始化羁绊配置
    std::vector<TraitConfig> allTraitConfigs = initTraitConfigs();

    // 2. 读取弈子文件（请替换为你的文件路径）
    std::string filePath = "tft_champions_trait.txt";  // 相对路径（和程序同目录）
    // 如果文件不在同目录，请用绝对路径，例如：
    // std::string filePath = "C:/Users/XXX/Documents/tft_champions_trait.txt";  // Windows
    // std::string filePath = "/home/XXX/tft_champions_trait.txt";  // Linux/macOS

    std::vector<Champion> allChampions = readChampionsFromFile(filePath);
    if (allChampions.empty()) {
        std::cerr << "错误：未读取到任何弈子信息，程序退出" << std::endl;
        return 1;
    }

    // 3. 输入人口数
    int population;
    std::cout << "\n请输入阵容人口数：";
    std::cin >> population;
    std::cin.ignore();  // 忽略回车

    // 4. 输入弈子名称（一行，空格分隔，支持别称）
    std::string championInput;
    std::cout << "请输入阵容弈子名称（空格分隔，支持别称）：";
    std::getline(std::cin, championInput);

    // 解析输入的弈子名称
    std::vector<std::string> inputChampions;
    std::stringstream ss(championInput);
    std::string name;
    while (ss >> name) {
        inputChampions.push_back(name);
    }

    // 5. 统计阵容中各羁绊的弈子数
    std::map<std::string, int> traitCountMap;
    for (const auto& inputName : inputChampions) {
        // 查找该名称（或别称）对应的弈子
        const Champion* champ = findChampionByAnyName(allChampions, inputName);
        if (champ != nullptr) {
            // 统计该弈子的所有羁绊
            for (const auto& trait : champ->traits) {
                traitCountMap[trait]++;
            }
            std::cout << "已识别弈子：" << inputName << " → 羁绊：";
            for (size_t i = 0; i < champ->traits.size(); i++) {
                if (i > 0) std::cout << "、";
                std::cout << champ->traits[i];
            }
            std::cout << std::endl;
        } else {
            std::cerr << "警告：未找到弈子[" << inputName << "]的信息，已忽略！" << std::endl;
        }
    }

    // 6. 生成所有羁绊的结果（包括未达成的）
    std::vector<TraitResult> traitResults;
    // 先处理有统计的羁绊（达成/部分达成）
    for (const auto& pair : traitCountMap) {
        TraitConfig config = getTraitConfig(allTraitConfigs, pair.first);
        TraitLevel level = judgeTraitLevel(config, pair.second);
        traitResults.push_back({
            pair.first,
            pair.second,
            level,
            level != TraitLevel::None
        });
    }
    // 补充所有预设的、未出现在阵容中的羁绊（未达成）
    for (const auto& config : allTraitConfigs) {
        if (traitCountMap.find(config.name) == traitCountMap.end()) {
            traitResults.push_back({
                config.name,
                0,
                TraitLevel::None,
                false
            });
        }
    }

    // 7. 按规则排序
    std::sort(traitResults.begin(), traitResults.end(), compareTraitResult);

    // 8. 输出结果
    std::cout << "\n===== 阵容羁绊分析结果 =====" << std::endl;
    for (const auto& result : traitResults) {
        std::string color = getLevelColor(result.level);
        std::string levelText = getLevelText(result.level);
        // 输出格式：【羁绊名称】- 已上阵：X个 - 等级：XXX
        std::cout << color 
                  << "【" << result.name << "】- 已上阵：" << result.count 
                  << "个 - 等级：" << levelText 
                  << COLOR_RESET << std::endl;
    }

    return 0;
}