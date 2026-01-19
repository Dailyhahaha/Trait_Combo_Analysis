#include <iostream>
#include <vector>
#include <cctype>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem> // C++17及以上支持
#include <windows.h>  
#include <limits>

// 基础颜色定义（兼容Windows/Linux/macOS）
#define COLOR_RESET "\033[0m"   // 重置颜色
#define COLOR_WHITE "\033[37m"  // 1费-白色
#define COLOR_GREEN "\033[32m"  // 2费-绿色
#define COLOR_BLUE "\033[34m"   // 3费-蓝色
#define COLOR_PURPLE "\033[35m" // 4费-紫色
#define COLOR_GOLD "\033[33m"   // 5/7费-金色
#define COLOR_RED "\033[31m"    // 专属独羁绊-红色

// 羁绊等级颜色
#define COLOR_BROWN "\033[38;5;94m"   // 黄铜色
#define COLOR_SILVER "\033[38;5;248m" // 银色
#define COLOR_YELLOW "\033[33m"       // 黄金色
#define COLOR_PINK "\033[35m"         // 棱彩色

// 全局标志，用于main函数控制循环是否继续
volatile sig_atomic_t keep_running = 1;

// 信号处理函数：捕获Ctrl+C（SIGINT信号）
void handle_sigint(int sig) {
    (void)sig;  // 避免未使用参数的警告
    keep_running = 0;  // 将标志置为0，终止循环
    std::cout << "\n接收到Ctrl+C，程序即将退出..." << std::endl;
}

// 羁绊等级枚举（删除冗余的 RED 枚举）
enum class TraitLevel
{
    None,     // 未达成
    Bronze,   // 黄铜
    Silver,   // 白银
    Gold,     // 黄金
    Prismatic // 棱彩
};

// 羁绊类型枚举
enum class TraitType
{
    Exclusive, // 专属独羁绊（星界游神、解脱者等）
    Normal,    // 普通羁绊（护卫、战士等）
    City       // 城邦羁绊（德玛西亚、艾欧尼亚等，有棱彩）
};

// 羁绊配置结构体
struct TraitConfig
{
    std::string name;                // 羁绊名称
    std::vector<int> requiredCounts; // 各等级所需弈子数
    bool hasPrismatic;               // 是否有棱彩级
    TraitType type;                  // 羁绊类型
};

// 弈子结构体
struct Champion
{
    std::vector<std::string> names;  // 名称+别称
    int cost;                        // 弈子费用（1/2/3/4/5/7）
    std::vector<std::string> traits; // 所属羁绊
};

// 最终输出的羁绊信息（字段顺序调整，避免初始化歧义）
struct TraitResult
{
    std::string name;
    int count;
    TraitLevel level;
    bool isAchieved;
    TraitType type; // 羁绊类型
};

// 存储输入的弈子
struct InputChampion
{
    std::string inputName; // 用户输入的名称/别称
    std::string realName;  // 弈子正式名称
    int cost;              // 费用
};

// 预设羁绊配置规则
std::vector<TraitConfig> initTraitConfigs()
{
    std::vector<TraitConfig> configs;
    // 有棱彩的城邦羁绊
    configs.push_back({"德玛西亚", {3, 5, 7, 11}, true, TraitType::City});
    configs.push_back({"艾欧尼亚", {3, 5, 7, 10}, true, TraitType::City});
    configs.push_back({"约德尔人", {2, 4, 6, 8, 10}, true, TraitType::City});
    configs.push_back({"诺克萨斯", {3, 5, 7, 10}, true, TraitType::City});
    configs.push_back({"恕瑞玛", {1, 2, 3, 4}, true, TraitType::City});
    configs.push_back({"比尔吉沃特", {3, 5, 7, 10}, true, TraitType::City});
    // 普通城邦羁绊
    configs.push_back({"弗雷尔卓德", {3, 5, 7}, false, TraitType::City});
    configs.push_back({"皮尔特沃夫", {2, 4, 6}, false, TraitType::City});
    configs.push_back({"祖安", {3, 5, 7}, false, TraitType::City});
    configs.push_back({"暗影岛", {2, 3, 4, 5}, false, TraitType::City});
    configs.push_back({"以绪塔尔", {3, 5, 7}, false, TraitType::City});
    configs.push_back({"巨神峰", {1, 2, 4, 6}, false, TraitType::City});
    configs.push_back({"虚空", {2, 4, 6, 9}, false, TraitType::City});
    // 普通羁绊
    configs.push_back({"护卫", {2, 4, 6}, false, TraitType::Normal});
    // configs.push_back({"战士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"法师", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"枪手", {2, 4}, false, TraitType::Normal});
    // configs.push_back({"射手", {2, 4, 6}, false, TraitType::Normal});
    // configs.push_back({"坦克", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"神谕者", {2, 4}, false, TraitType::Normal});
    configs.push_back({"狙神", {2, 3, 4, 5}, false, TraitType::Normal});
    configs.push_back({"裁决战士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"主宰", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"迅击战士", {2, 3, 4, 5}, false, TraitType::Normal});
    configs.push_back({"斗士", {2, 4, 6}, false, TraitType::Normal});
    configs.push_back({"耀光使", {2, 4}, false, TraitType::Normal});
    configs.push_back({"神盾使", {2, 3, 4, 5}, false, TraitType::Normal});
    configs.push_back({"征服者", {2, 3, 4, 5}, false, TraitType::Normal});
    configs.push_back({"暗裔", {1, 2, 3}, false, TraitType::Normal});
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

// 从文件读取弈子信息
std::vector<Champion> readChampionsFromFile(const std::string &filePath)
{
    std::vector<Champion> champions;
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        std::cerr << "错误：无法打开文件 " << filePath << std::endl;
        return champions;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line))
    {
        lineNum++;
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;

        while (ss >> token)
        {
            parts.push_back(token);
        }

        if (parts.size() < 3)
        {
            std::cerr << "警告：第" << lineNum << "行格式错误（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        Champion champ;
        // 解析名称/别称
        std::string namePart = parts[0];
        std::stringstream nameSS(namePart);
        std::string name;
        while (std::getline(nameSS, name, '/'))
        {
            name.erase(std::remove_if(name.begin(), name.end(), isspace), name.end());
            if (!name.empty())
            {
                champ.names.push_back(name);
            }
        }

        // 解析费用
        try
        {
            champ.cost = std::stoi(parts[1]);
        }
        catch (...)
        {
            std::cerr << "警告：第" << lineNum << "行费用格式错误（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        // 解析羁绊
        for (size_t i = 2; i < parts.size(); i++)
        {
            champ.traits.push_back(parts[i]);
        }

        if (champ.names.empty() || champ.traits.empty())
        {
            std::cerr << "警告：第" << lineNum << "行数据无效（内容：" << line << "），已跳过" << std::endl;
            continue;
        }

        champions.push_back(champ);
    }

    file.close();
    std::cout << "成功读取 " << champions.size() << " 个弈子信息" << std::endl;
    return champions;
}

// 根据名称/别称查找弈子
const Champion *findChampionByAnyName(const std::vector<Champion> &champions, const std::string &inputName, std::string &realName)
{
    for (const auto &champ : champions)
    {
        for (size_t i = 0; i < champ.names.size(); i++)
        {
            if (champ.names[i] == inputName)
            {
                realName = champ.names[0];
                return &champ;
            }
        }
    }
    realName = "";
    return nullptr;
}

// 获取费用对应的颜色
std::string getCostColor(int cost)
{
    switch (cost)
    {
    case 1:
        return COLOR_WHITE;
    case 2:
        return COLOR_GREEN;
    case 3:
        return COLOR_BLUE;
    case 4:
        return COLOR_PURPLE;
    case 5:
    case 7:
        return COLOR_GOLD;
    default:
        return COLOR_RESET;
    }
}

// 获取羁绊结果的显示颜色（专属羁绊红色，普通羁绊按等级）
std::string getResultColor(const TraitResult &result)
{
    // 专属独羁绊（达成）→ 红色
    if (result.type == TraitType::Exclusive && result.isAchieved)
    {
        return COLOR_RED;
    }
    if (result.name == "巨神峰")
    {
        return COLOR_YELLOW;
    }
    // 普通羁绊按等级取色
    switch (result.level)
    {
    case TraitLevel::Bronze:
        return COLOR_BROWN;
    case TraitLevel::Silver:
        return COLOR_SILVER;
    case TraitLevel::Gold:
        return COLOR_YELLOW;
    case TraitLevel::Prismatic:
        return COLOR_PINK;
    default:
        return COLOR_RESET;
    }
}

// 获取羁绊配置（补充 type 字段）
TraitConfig getTraitConfig(const std::vector<TraitConfig> &configs, const std::string &traitName)
{
    for (const auto &config : configs)
    {
        if (config.name == traitName)
        {
            return config;
        }
    }
    // 未找到的羁绊默认归为普通类型
    return {traitName, {}, false, TraitType::Normal};
}

// 判定羁绊等级
TraitLevel judgeTraitLevel(const TraitConfig &config, int count)
{
    if (config.requiredCounts.empty() || count < config.requiredCounts[0])
    {
        return TraitLevel::None;
    }

    if (config.hasPrismatic)
    {
        if (count >= config.requiredCounts.back())
        {
            return TraitLevel::Prismatic;
        }
        else if (count >= config.requiredCounts[2])
        {
            return TraitLevel::Gold;
        }
        else if (count >= config.requiredCounts[1])
        {
            return TraitLevel::Silver;
        }
        else if (count >= config.requiredCounts[0])
        {
            return TraitLevel::Bronze;
        }
    }
    else
    {
        if (count >= config.requiredCounts.back())
        {
            return TraitLevel::Gold;
        }
        else if (config.requiredCounts.size() >= 2 && count >= config.requiredCounts[1])
        {
            return TraitLevel::Silver;
        }
        else if (count >= config.requiredCounts[0])
        {
            return TraitLevel::Bronze;
        }
    }
    return TraitLevel::None;
}

// 排序规则：棱彩城邦 > 专属羁绊 > 普通城邦 > 普通羁绊；同类型按等级/名称
bool compareTraitResult(const TraitResult &a, const TraitResult &b)
{
    // 1. 达成的羁绊优先
    if (a.isAchieved != b.isAchieved)
    {
        return a.isAchieved > b.isAchieved;
    }

    // 2. 棱彩级城邦羁绊优先级最高
    bool aIsPrismaticCity = (a.type == TraitType::City && a.level == TraitLevel::Prismatic);
    bool bIsPrismaticCity = (b.type == TraitType::City && b.level == TraitLevel::Prismatic);
    if (aIsPrismaticCity != bIsPrismaticCity)
    {
        return aIsPrismaticCity > bIsPrismaticCity;
    }

    // 3. 专属羁绊次之
    if (a.type != b.type)
    {
        if (a.type == TraitType::Exclusive)
            return true;
        if (b.type == TraitType::Exclusive)
            return false;
        // 城邦羁绊 > 普通羁绊
        return a.type == TraitType::City && b.type == TraitType::Normal;
    }

    // 4. 同类型按等级从高到低
    if (a.level != b.level)
    {
        return static_cast<int>(a.level) > static_cast<int>(b.level);
    }

    // 5. 等级相同按名称排序
    return a.name < b.name;
}

// 获取等级文本描述
std::string getLevelText(TraitLevel level)
{
    switch (level)
    {
    case TraitLevel::Bronze:
        return "黄铜级";
    case TraitLevel::Silver:
        return "白银级";
    case TraitLevel::Gold:
        return "黄金级";
    case TraitLevel::Prismatic:
        return "棱彩级";
    default:
        return "未达成";
    }
}

int main()
{
    // 1. 初始化配置和读取弈子
    std::vector<TraitConfig> allTraitConfigs = initTraitConfigs();
    std::string filePath = "../Trait_information/tft_champions_trait_GB2312.txt";
    std::vector<Champion> allChampions = readChampionsFromFile(filePath);
    if (allChampions.empty())
    {
        std::cerr << "错误：未读取到任何弈子信息，程序退出" << std::endl;
        return 1;
    }
    int n = 0;
    char input_char;
    while (keep_running)
    {
        // 2. 输入人口数
        int population;
        std::cout << "\n请输入阵容人口数：";
        std::cin >> population;
        std::cin.ignore();

        // 3. 输入弈子名称
        std::string championInput;
        std::cout << "请输入阵容弈子名称（空格分隔，支持别称）：";
        std::getline(std::cin, championInput);

        // 4. 解析输入并统计
        std::vector<InputChampion> inputChamps;
        std::map<std::string, int> traitCountMap;
        std::stringstream ss(championInput);
        std::string name;

        while (ss >> name)
        {
            std::string realName;
            const Champion *champ = findChampionByAnyName(allChampions, name, realName);
            if (champ != nullptr)
            {
                // 统计羁绊
                for (const auto &trait : champ->traits)
                {
                    traitCountMap[trait]++;
                }
                // 记录输入弈子
                inputChamps.push_back({name, realName, champ->cost});
                // 输出识别结果
                std::cout << "已识别弈子：" << name << "（正式名：" << realName << "，费用：" << champ->cost << "费）→ 羁绊：";
                for (size_t i = 0; i < champ->traits.size(); i++)
                {
                    if (i > 0)
                        std::cout << "、";
                    std::cout << champ->traits[i];
                }
                std::cout << std::endl;
            }
            else
            {
                std::cerr << "警告：未找到弈子[" << name << "]的信息，已忽略！" << std::endl;
            }
        }

        // 5. 彩色输出阵容弈子
        std::cout << "\n===== 阵容弈子列表 =====" << std::endl;
        std::cout << "当前阵容弈子：";
        for (size_t i = 0; i < inputChamps.size(); i++)
        {
            if (i > 0)
                std::cout << " ";
            std::string color = getCostColor(inputChamps[i].cost);
            std::cout << color << inputChamps[i].inputName << COLOR_RESET;
        }
        std::cout << std::endl;

        // 6. 生成羁绊结果（补充 type 字段，核心修复）
        std::vector<TraitResult> traitResults;
        for (const auto &pair : traitCountMap)
        {
            TraitConfig config = getTraitConfig(allTraitConfigs, pair.first);
            TraitLevel level = judgeTraitLevel(config, pair.second);
            // 完整初始化 TraitResult（包含 type 字段）
            traitResults.push_back({
                pair.first,
                pair.second,
                level,
                level != TraitLevel::None,
                config.type // 关键：传入羁绊类型
            });
        }

        // 7. 按优先级排序
        std::sort(traitResults.begin(), traitResults.end(), compareTraitResult);

        // 8. 输出羁绊结果（恢复等级文本，颜色正常生效）
        std::cout << "\n===== 阵容羁绊分析结果 =====" << std::endl;
        for (const auto &result : traitResults)
        {
            std::string color = getResultColor(result);
            std::string levelText = getLevelText(result.level);
            std::cout << color
                      << "【" << result.name << "】:" << result.count
                      << COLOR_RESET << std::endl;
        }       

        // ========== 循环末尾的判定逻辑 ==========
        bool valid_input = false; // 标记输入是否有效
        while (!valid_input) {    // 输入无效则循环等待正确输入
            std::cout << "请输入指令（c=继续，q=退出）：";
            // 读取用户输入的第一个字符
            std::cin >> input_char;
            // 清空输入缓冲区，避免残留字符干扰
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // 统一转为小写后判断
            char lower_char = tolower(input_char);
            if (lower_char == 'q') {
                keep_running = false; // 退出循环
                valid_input = true;   // 标记输入有效，退出输入判定循环
                std::cout << "检测到退出指令，程序即将结束..." << std::endl;
            } else if (lower_char == 'c') {
                valid_input = true;   // 标记输入有效，继续主循环
                //std::cout << "继续执行下一次循环..." << std::endl;
                system("cls"); // 清屏，准备下一次输入
            } else {
                // 输入非法字符，提示并重新输入
                std::cout << "输入错误！请输入 c（继续）或 q（退出）\n" << std::endl;
            }
        }
    }
    std::cout << "程序已正常退出" << std::endl;
    return 0;
}