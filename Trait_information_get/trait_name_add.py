import os  # 必须导入os模块

# 1. 基础用法：获取并打印当前工作目录
#current_working_dir = os.getcwd()
#print("当前工作目录（CWD）：", current_working_dir)

# 1. 获取当前Python文件的绝对路径
current_file_path = os.path.abspath(__file__)
# 2. 提取文件所在的文件夹路径（这就是你要同步的目标目录）
current_file_dir = os.path.dirname(current_file_path)
# 3. 切换工作目录到文件所在目录
os.chdir(current_file_dir)

def load_champions_data(file_path):
    """加载原始弈子数据，返回列表：每个元素为 [名称, 羁绊列表]"""
    champions = []
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                # 分割名称和羁绊（第一个空格前是名称/别名部分，后面是羁绊）
                parts = line.split(" ", 1)
                name_alias_part = parts[0]
                # 提取纯名称（忽略已存在的别名）
                pure_name = name_alias_part.split("/")[0]
                traits = parts[1].split(" ") if len(parts) > 1 else []
                champions.append([pure_name, traits])
        return champions
    except FileNotFoundError:
        print(f"错误：未找到文件 {file_path}")
        exit(1)

def save_champions_data(file_path, champions, aliases_dict):
    """直接保存到原始文件（覆盖），格式：名称/别名1/别名2 羁绊1 羁绊2..."""
    try:
        with open(file_path, "w", encoding="utf-8") as f:
            for name, traits in champions:
                # 获取该弈子的所有别名，去重并保持顺序
                alias_list = aliases_dict.get(name, [])
                unique_aliases = []
                for alias in alias_list:
                    if alias not in unique_aliases and alias != name:
                        unique_aliases.append(alias)
                # 构建名称+别名部分
                name_alias_part = "/".join([name] + unique_aliases)
                # 构建羁绊部分
                trait_part = " ".join(traits)
                # 写入文件
                f.write(f"{name_alias_part} {trait_part}\n" if trait_part else f"{name_alias_part}\n")
        print(f"\n数据已成功保存到原始文件：{file_path}")
    except Exception as e:
        print(f"错误：保存文件失败 - {str(e)}")
        exit(1)

def add_specified_aliases(champions):
    """功能1：添加指定弈子的别名"""
    aliases_dict = {}
    # 获取所有弈子纯名称（用于验证输入是否存在）
    champion_names = [champ[0] for champ in champions]
    
    try:
        count = int(input("请输入要修改的弈子数量："))
        if count <= 0:
            print("数量必须为正整数！")
            return aliases_dict
    except ValueError:
        print("输入无效，请输入整数！")
        return aliases_dict
    
    for i in range(count):
        print(f"\n第 {i+1} 个弈子：")
        # 输入名称和别名（格式：名称/别名1/别名2...）
        input_str = input("请输入弈子名称和别名（用 '/' 分隔）：").strip()
        if not input_str:
            print("输入不能为空，跳过该弈子！")
            continue
        
        parts = input_str.split("/")
        champion_name = parts[0].strip()
        
        # 验证弈子是否存在
        if champion_name not in champion_names:
            print(f"警告：弈子 '{champion_name}' 不存在于原始数据中，跳过该条目！")
            continue
        
        # 提取别名（去空）
        aliases = [alias.strip() for alias in parts[1:] if alias.strip()]
        if not aliases:
            print(f"未输入有效别名，'{champion_name}' 无新增别名！")
            continue
        
        # 存储别名（支持多个别名）
        aliases_dict[champion_name] = aliases
        print(f"成功添加：{champion_name} -> 别名：{', '.join(aliases)}")
    
    return aliases_dict

def modify_all_aliases(champions):
    """功能2：逐个询问所有弈子的别名"""
    aliases_dict = {}
    total = len(champions)
    print(f"\n共需处理 {total} 个弈子，按 Enter 可快速跳过无别名的弈子\n")
    
    for idx, (name, traits) in enumerate(champions, 1):
        print(f"[{idx}/{total}] 弈子：{name}")
        # 询问是否有别名
        has_alias = input("是否有别名？（y/n，直接回车视为n）：").strip().lower()
        
        if has_alias != "y":
            print(f"→ 无别名，跳过\n")
            continue
        
        # 输入别名（多个用 '/' 分隔）
        alias_input = input("请输入所有别名（用 '/' 分隔）：").strip()
        if not alias_input:
            print(f"→ 未输入有效别名，跳过\n")
            continue
        
        # 提取别名（去空）
        aliases = [alias.strip() for alias in alias_input.split("/") if alias.strip()]
        if not aliases:
            print(f"→ 无有效别名，跳过\n")
            continue
        
        # 存储别名
        aliases_dict[name] = aliases
        print(f"→ 已记录别名：{', '.join(aliases)}\n")
    
    return aliases_dict

def main():
    # 原始文件路径（可根据实际情况修改）
    FILE_PATH = "../Trait_information/tft_champions_trait.txt"
    
    print("===== 云顶之弈S16弈子别名管理工具（直接修改原始文件）=====")
    print("⚠️  警告：此操作会直接覆盖原始文件，建议先备份文件！")
    print("操作选项：")
    print("1 - 添加指定几个弈子的别名信息")
    print("2 - 逐个修改所有弈子的别名信息")
    
    # 选择操作
    while True:
        choice = input("\n请输入操作编号（1/2）：").strip()
        if choice in ["1", "2"]:
            break
        print("输入无效，请重新选择！")
    
    # 加载原始数据
    print("\n正在加载弈子数据...")
    champions = load_champions_data(FILE_PATH)
    print(f"成功加载 {len(champions)} 个弈子数据")
    
    # 执行对应操作
    if choice == "1":
        aliases_dict = add_specified_aliases(champions)
    else:
        aliases_dict = modify_all_aliases(champions)
    
    # 确认保存（增加安全校验）
    while True:
        confirm = input("\n是否确认保存到原始文件？（y/n，默认n）：").strip().lower()
        if confirm in ["y", "n", ""]:
            break
        print("输入无效，请输入 y 或 n！")
    
    if confirm == "y":
        save_champions_data(FILE_PATH, champions, aliases_dict)
        print("\n操作完成！")
    else:
        print("\n已取消保存，未修改原始文件！")

if __name__ == "__main__":
    main()