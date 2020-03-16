#coding=utf-8

import os
import json

#获取目标文件夹的路径
filedir = os.getcwd()+'/json'
#获取文件夹中的文件名称列表
filenames=os.listdir(filedir)
#遍历文件名
for filename in filenames:
    filepath = filedir+'/'+filename
    print filepath
    name=filename.split('.')[0]
    jpgname=name+'.jpg'
    after = []
    # 打开文件取出数据并修改，然后存入变量
    with open(filepath, 'rb') as f:
        data = json.load(f)
        print type(data)
        data['imagePath']=jpgname
        after = data

    # 打开文件并覆盖写入修改后内容
    with open(filepath, 'wb') as f:
        data = json.dump(after, f)
