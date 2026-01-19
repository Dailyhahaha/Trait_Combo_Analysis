import json
import os

# 1. 获取当前Python文件的绝对路径
current_file_path = os.path.abspath(__file__)
# 2. 提取文件所在的文件夹路径（这就是你要同步的目标目录）
current_file_dir = os.path.dirname(current_file_path)
# 3. 切换工作目录到文件所在目录
os.chdir(current_file_dir)

# 读取JSON文件
with open("../Trait_information/tft_champions_trait.json", "r", encoding="utf-8") as f:
    data = json.load(f)

# 遍历每个英雄数据，修改字段
for champion in data:
    # 获取英雄名称（处理特殊名称，比如带"/"、"与"等的情况，确保文件名合法）
    champion_name = champion["名称"]
    # 替换可能影响文件名的特殊字符（可选，根据实际需求调整）
    safe_name = champion_name.replace("与", "_").replace("·", "_").replace(" ", "")
    # 新增/替换头像路径字段
    champion["头像路径"] = f"../images/{safe_name}.jpg"
    # 删除原有的头像URL字段（如果需要保留可以注释这行）
    if "头像URL" in champion:
        del champion["头像URL"]

# 将修改后的数据写回文件
with open("./tft_champions_trait_modified.json", "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, indent=2)

print("文件修改完成，新文件为 tft_champions_trait_modified.json")