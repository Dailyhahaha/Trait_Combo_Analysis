import sys
import json
import os
# 完整且正确的导入（新增 QIcon）
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QHBoxLayout, 
                             QVBoxLayout, QListWidget, QListWidgetItem, QTextEdit)
from PyQt6.QtGui import QPixmap, QDrag, QColor, QTextCharFormat, QFont, QIcon  # 关键：导入 QIcon
from PyQt6.QtCore import Qt, QMimeData, QSize

# ===================== 核心修正：适配你的 JSON 格式 + 路径修复 =====================
def load_champion_data():
    """加载并处理弈子数据，返回 名称->数据 的字典"""
    # 获取当前脚本所在目录（绝对路径）
    current_dir = os.path.dirname(os.path.abspath(__file__))
    # JSON 文件路径（和脚本同目录）
    json_file_path = os.path.join(current_dir, "champions.json")
    
    try:
        with open(json_file_path, "r", encoding="utf-8") as f:
            champion_list = json.load(f)
        
        champion_data = {}
        for item in champion_list:
            champ_name = item.get("名称")
            if not champ_name:
                print("警告：发现无名称的弈子数据，已跳过")
                continue
            # 修复头像路径：将 ../images/xxx.jpg 转为绝对路径
            img_path = item.get("头像路径", "")
            if img_path:
                # 拼接正确的图片绝对路径（适配 ../images 格式）
                img_abs_path = os.path.normpath(os.path.join(current_dir, img_path))
                item["头像路径_绝对"] = img_abs_path
            champion_data[champ_name] = item
        
        if not champion_data:
            print("错误：JSON 文件中未找到有效弈子数据！")
            sys.exit(1)
        
        print(f"成功加载 {len(champion_data)} 个弈子数据")
        return champion_data
    
    except FileNotFoundError:
        print(f"错误：未找到 JSON 文件，请检查路径：{json_file_path}")
        sys.exit(1)
    except json.JSONDecodeError:
        print("错误：JSON 文件格式错误，请检查文件内容")
        sys.exit(1)
    except Exception as e:
        print(f"错误：加载弈子数据失败 - {str(e)}")
        sys.exit(1)

# 加载弈子数据
champion_data = load_champion_data()

# 城邦羁绊激活条件
city_traits = {
    "德玛西亚": 3, "艾欧尼亚": 3, "约德尔人": 2, "诺克萨斯": 3,
    "恕瑞玛": 2, "虚空": 2, "巨神峰": 1, "比尔吉沃特": 3,
    "皮尔特沃夫": 2, "弗雷尔卓德": 3, "祖安": 3, "暗影岛": 2
}

class ChampionItem(QListWidgetItem):
    """自定义弈子列表项（含图片+名称）- 修复 QIcon/QPixmap 错误"""
    def __init__(self, name, image_abs_path):
        super().__init__()
        self.champion_name = name
        # 修复图片加载逻辑：QPixmap -> QIcon
        pixmap = QPixmap()
        if image_abs_path and os.path.exists(image_abs_path):
            pixmap.load(image_abs_path)
        else:
            print(f"警告：图片不存在，路径：{image_abs_path}")
        
        if pixmap.isNull():
            self.setText(name)
            self.setSizeHint(QSize(80, 100))
        else:
            # 缩放图片
            pixmap = pixmap.scaled(
                80, 100, 
                Qt.AspectRatioMode.KeepAspectRatio, 
                Qt.TransformationMode.SmoothTransformation
            )
            # 关键修复：将 QPixmap 包装为 QIcon 再设置
            self.setIcon(QIcon(pixmap))  # 不再直接传 QPixmap
            self.setText(name)
            self.setSizeHint(pixmap.size())

class TFTMainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("云顶之弈羁绊计算器")
        self.setGeometry(100, 100, 1200, 600)
        self.selected_champions = []
        self.init_ui()
        self.init_drag_drop()  # 初始化拖拽事件

    def init_ui(self):
        main_layout = QHBoxLayout()
        
        # 1. 左侧：弈子池（可拖拽）
        self.champion_pool = QListWidget()
        self.champion_pool.setViewMode(QListWidget.ViewMode.IconMode)
        self.champion_pool.setDragEnabled(True)
        self.champion_pool.setDefaultDropAction(Qt.DropAction.CopyAction)
        self.champion_pool.setIconSize(QSize(80, 100))
        self.champion_pool.setSpacing(10)
        
        # 填充弈子池（使用绝对路径加载图片）
        for name, data in champion_data.items():
            image_abs_path = data.get("头像路径_绝对", "")
            item = ChampionItem(name, image_abs_path)
            self.champion_pool.addItem(item)
        
        # 2. 中间：已选弈子（接收拖拽）
        self.selected_pool = QListWidget()
        self.selected_pool.setViewMode(QListWidget.ViewMode.IconMode)
        self.selected_pool.setAcceptDrops(True)
        self.selected_pool.setDropIndicatorShown(True)
        self.selected_pool.setIconSize(QSize(80, 100))
        self.selected_pool.setSpacing(10)
        self.selected_pool.itemClicked.connect(self.on_remove_item)
        
        # 3. 右侧：统计面板
        self.stats_panel = QTextEdit()
        self.stats_panel.setReadOnly(True)
        self.stats_panel.setFont(QFont("微软雅黑", 10))
        
        main_layout.addWidget(self.champion_pool, 2)
        main_layout.addWidget(self.selected_pool, 1)
        main_layout.addWidget(self.stats_panel, 2)
        
        central_widget = QWidget()
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

    # 修复：正确处理拖拽放下事件
    def dropEvent(self, event):
        if event.source() != self.champion_pool:
            event.ignore()
            return
        
        mime_data = event.mimeData()
        if not mime_data or not mime_data.hasText():
            event.ignore()
            return
        
        champ_name = mime_data.text().strip()
        if champ_name not in champion_data:
            event.ignore()
            return
        
        # 添加到已选池
        data = champion_data[champ_name]
        image_abs_path = data.get("头像路径_绝对", "")
        item = ChampionItem(champ_name, image_abs_path)
        self.selected_pool.addItem(item)
        
        self.update_stats()
        event.acceptProposedAction()

    # 初始化拖拽事件绑定
    def init_drag_drop(self):
        self.selected_pool.dropEvent = self.dropEvent

    # 点击删除已选弈子
    def on_remove_item(self, item):
        row = self.selected_pool.row(item)
        if row >= 0:
            self.selected_pool.takeItem(row)
            self.update_stats()

    # 实时更新羁绊统计
    def update_stats(self):
        self.selected_champions.clear()
        for i in range(self.selected_pool.count()):
            item = self.selected_pool.item(i)
            if hasattr(item, "champion_name"):
                self.selected_champions.append(item.champion_name)
        
        trait_count = {}
        for name in self.selected_champions:
            traits = champion_data[name].get("羁绊", [])
            for t in traits:
                trait_count[t] = trait_count.get(t, 0) + 1
        
        # 生成统计文本（HTML 富文本）
        stats_text = f"<b>已选弈子数量：{len(self.selected_champions)}</b><br><br>"
        stats_text += "<b>【城邦羁绊】</b><br>"
        
        has_activated = False
        has_unactivated = False
        for trait, need in city_traits.items():
            count = trait_count.get(trait, 0)
            if count >= need:
                stats_text += f'<font color="red">[已激活] {trait}: {count} (需{need})</font><br>'
                has_activated = True
                trait_count[trait] = -1
            elif count > 0:
                stats_text += f"[未激活] {trait}: {count} (需{need})<br>"
                has_unactivated = True
                trait_count[trait] = -1
        
        if not has_activated and not has_unactivated:
            stats_text += "无城邦羁绊<br>"
        
        stats_text += "<br><b>【其他羁绊】</b><br>"
        has_other = False
        for trait, count in trait_count.items():
            if count > 0:
                stats_text += f"{trait}: {count}<br>"
                has_other = True
        if not has_other:
            stats_text += "无其他羁绊<br>"
        
        self.stats_panel.setHtml(stats_text)

# 修复：PyQt6 正确重写 startDrag 方法
def custom_start_drag(self, supported_actions):
    item = self.currentItem()
    if not item or not hasattr(item, "champion_name"):
        return
    
    mime_data = QMimeData()
    mime_data.setText(item.champion_name)
    
    drag = QDrag(self)
    drag.setMimeData(mime_data)
    
    # 拖拽预览图片（仍用 QPixmap）
    if not item.icon().isNull():
        drag.setPixmap(item.icon().pixmap(80, 100))
    
    # PyQt6 中指定拖拽行为
    drag.exec(Qt.DropAction.CopyAction)

# 替换默认的 startDrag 方法
QListWidget.startDrag = custom_start_drag

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setFont(QFont("微软雅黑"))  # 解决中文显示问题
    
    window = TFTMainWindow()
    window.show()
    
    sys.exit(app.exec())