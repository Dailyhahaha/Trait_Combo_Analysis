# 代码介绍
文件夹包含两个代码.

## tft_trait_extractor.py
其中`tft_trait_extractor.py`用于在云顶之弈官方网站中获取弈子羁绊信息,收集到的信息保存在`../Trait_information/tft_champions_trait.txt`以及`../Trait_information/tft_champions_trait.json`

对于前者,弈子信息以**弈子名称 羁绊一 羁绊二**的形式保存,此处弈子信息没有别名.

## trait_name_add.py

该代码用于给弈子添加别名信息,直接在`../Trait_information/tft_champions_trait.txt`中改动.改动后弈子信息以**弈子名称/别名 羁绊一 羁绊二**的形式保存