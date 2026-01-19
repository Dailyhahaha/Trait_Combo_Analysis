from selenium import webdriver
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.by import By
from bs4 import BeautifulSoup
import time
import json
import requests

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

if(os.getcwd() == current_file_dir):
    print("工作目录已同步到当前文件所在目录：", os.getcwd())
else:
    print("工作目录同步失败，当前工作目录仍为：", os.getcwd())
# 4. 验证：打印切换后的工作目录，确认同步成功
#print("同步后的工作目录：", os.getcwd())
#print("当前文件所在目录：", current_file_dir)
#print("是否同步成功：", os.getcwd() == current_file_dir)

def crawl_tft_champions_trait():
    # 1. 初始化浏览器（适配Selenium 4.x）
    options = webdriver.ChromeOptions()
    options.add_argument('--headless=new')  # 可选：无头模式，隐藏浏览器
    # options.add_argument('--window-size=1920,1080')
    options.add_argument('--disable-blink-features=AutomationControlled')
    options.add_experimental_option('excludeSwitches', ['enable-automation'])
    
    from webdriver_manager.chrome import ChromeDriverManager
    from selenium.webdriver.chrome.service import Service
    service = Service(ChromeDriverManager().install())
    driver = webdriver.Chrome(service=service, options=options)
    
    # 2. 打开云顶之弈弈子页面
    url = "https://lol.qq.com/tft/#/champion"
    driver.get(url)
    wait = WebDriverWait(driver, 5)
    print("页面加载中，等待核心容器渲染...")

    try:
        # ========== 按你的DOM层级定位champion-list ==========
        champion_list = wait.until(
            EC.presence_of_element_located((By.CSS_SELECTOR, ".page-champion .champion-list"))
        )
        print("✅ 找到champion-list容器")
        time.sleep(3)  # 等待容器内所有champion-item-big完全渲染

        # ========== 解析整个页面的HTML（用于BeautifulSoup提取羁绊） ==========
        page_html = driver.page_source
        soup = BeautifulSoup(page_html, 'html.parser')
        # 找到所有champion-item-big（每个对应一个弈子）
        champion_items_big = soup.select(".champion-list .champion-item-big")
        if len(champion_items_big) == 0:
            print("⚠️ 未找到champion-item-big，检查class名称是否更新")
            driver.quit()
            return []
        print(f"✅ 共找到 {len(champion_items_big)} 个弈子容器")

        # ========== 遍历每个弈子，提取羁绊+基础信息 ==========
        tft_champions = []
        for idx, item_big in enumerate(champion_items_big, 1):
            try:
                champion_data = {}

                champion_name = item_big.find("div", class_="champion-item-info")
                if champion_name:
                    name_elem = champion_name.find("span", class_="name")
                    champion_data["名称"] = name_elem.get_text(strip=True) if name_elem else "未知"
                else:
                    champion_data["名称"] = "未知"

                # 提取弈子费用信息
                champion_price = item_big.find("span", class_="price")
                if champion_price:
                    champion_data["费用"] = champion_price.get_text(strip=True)
                else:
                    champion_data["费用"] = "未知"
                # 1. 提取羁绊信息（核心：按你的DOM层级）
                # 找到race-job-wrap → 所有race-job → 提取race-job-name的文本
                race_job_wrap = item_big.find("div", class_="race-job-wrap")
                if race_job_wrap:
                    # 提取所有羁绊名称
                    race_job_names = [
                        job.find("span", class_="race-job-name").get_text(strip=True)
                        for job in race_job_wrap.find_all("div", class_="race-job")
                        if job.find("span", class_="race-job-name")
                    ]
                    champion_data["羁绊"] = race_job_names if race_job_names else ["未知"]
                else:
                    champion_data["羁绊"] = ["未知"]

                # 2. 提取弈子名称（从背景图URL或其他位置补充，若需要）
                # 方式1：从champion-item-pic的background-image中提取（URL含弈子ID，可选）
                pic_elem = item_big.find("div", class_="champion-item-pic")
                if pic_elem and pic_elem.get("style"):
                    avatar_url = pic_elem.get("style").split("url(\"")[1].split("\")")[0]
                    champion_data["头像URL"] = avatar_url

                if avatar_url.startswith("//"):
                    avatar_url = "https:" + avatar_url
                
                # 提取弈子名称（假设已存在champion_data["名称"]）
                champion_name = champion_data["名称"]
                # 处理名称中的特殊字符（避免文件名非法，比如/、\、:等）
                safe_name = champion_name.replace("/", "").replace("\\", "").replace(":", "").replace("*", "").replace("?", "").replace("\"", "").replace("<", "").replace(">", "").replace("|", "")

                # 2. 下载并保存图片
                '''
                try:
                 # 发送请求获取图片（添加超时和请求头，模拟浏览器）
                    headers = {
                        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
                    }
                    response = requests.get(avatar_url, headers=headers, timeout=10)
                    response.raise_for_status()  # 检查请求是否成功
        
                    # 拼接保存路径
                    save_path = os.path.join("../image", f"{safe_name}.jpg")
        
                    # 写入图片文件
                    with open(save_path, "wb") as f:
                        f.write(response.content)
        
                    print(f"✅ 成功下载：{champion_name} -> {save_path}")

                except requests.exceptions.RequestException as e:
                    print(f"❌ 下载失败：{champion_name}，错误：{str(e)}")
                except Exception as e:
                    print(f"❌ 保存失败：{champion_name}，错误：{str(e)}")
                   '''
                # 3. 保存数据
                tft_champions.append(champion_data)
                print(f"✅ 已抓取 {idx}/{len(champion_items_big)}: {champion_data['名称']} → 羁绊：{champion_data['羁绊']}")

            except Exception as e:
                print(f"❌ 抓取第{idx}个弈子出错：{str(e)[:50]}...（跳过）")
                continue

    except Exception as e:
        print(f"❌ 初始化失败：{str(e)}")
        driver.quit()
        return []

    # ========== 保存JSON文件（原有功能） ==========
    driver.quit()
    with open("../Trait_information/tft_champions_trait.json", "w", encoding="utf-8") as f:
        json.dump(tft_champions, f, ensure_ascii=False, indent=2)
    
    # ========== 新增：处理数据并保存到TXT文件 ==========
    # 1. 准备TXT内容（格式：弈子名称 羁绊一 羁绊二 羁绊三）
    txt_content = []
    for champion in tft_champions:
        name = champion["名称"]
        price = champion["费用"]
        price_num = ''.join([c for c in price if c.isdigit()])  # 只保留数字字符
        traits = champion["羁绊"]
        # 拼接格式：名称 + 空格分隔的羁绊（不足3个也不补空，超过3个也保留）
        # 示例：璐璐 约德尔人 法师 | 金克丝 约德尔人 枪手 暴走萝莉
        line = f"{name} {price_num} {' '.join(traits)}"
        txt_content.append(line)
    
    # 2. 保存到TXT文件（UTF-8编码避免中文乱码）
    with open("../Trait_information/tft_champions_trait.txt", "w", encoding="utf-8") as f:
        # 每行一个弈子，换行符分隔
        f.write("\n".join(txt_content))
    
    print(f"\n🎉 抓取完成！")
    print(f"📄 JSON文件：tft_champions_trait.json（{len(tft_champions)}个弈子）")
    print(f"📄 TXT文件：tft_champions_trait.txt（格式：弈子名称 弈子费用 羁绊一 羁绊二 羁绊三）")

    return tft_champions

# 执行爬虫
if __name__ == "__main__":
    crawl_tft_champions_trait()