
# coding: utf-8

# In[46]:


import os
import cv2
import xml.etree.ElementTree as ET
import numpy as np


# In[ ]:


InputTextPath='/media/mahxn0/DATA/dataset/helmet_hand_data/hands.txt'
with open(InputTextPath,'r') as File_1:
    Lines=File_1.readlines()
    for Line in Lines:
        Line=Line.strip('\n')
        print(Line)
        ImgNames=Line[:-3]+'jpg'
        XmlNames=Line[:-3]+'xml'
        Img=cv2.imread(ImgNames)
        H,W=Img.shape[:2]
        with open(XmlNames,'w') as xml_file:
            xml_file.write('<annotation>\n')
            xml_file.write('    <folder>VOC2007</folder>\n')
            xml_file.write('    <filename>' + str(ImgNames) + '.png' + '</filename>\n')
            xml_file.write('    <size>\n')
            xml_file.write('        <width>' + str(W) + '</width>\n')
            xml_file.write('        <height>' + str(H) + '</height>\n')
            xml_file.write('        <depth>3</depth>\n')
            xml_file.write('    </size>\n')
            with open(Line,'r') as File_2:
                Txts=[Txt_a.rstrip('\n') for Txt_a in File_2.readlines()]
                for Txt in Txts:
                    Txt=Txt.split(' ')
                    xmin=int(float(Txt[1])*W)
                    ymin=int(float(Txt[2])*H)
                    xmax=int(xmin+float(Txt[3])*W)
                    ymax=int(ymin+float(Txt[4])*H)
                    print(xmin,xmax,ymin,ymax)
                    name='hand'
                    xml_file.write('    <object>\n')
                    xml_file.write('        <name>' + str(name) + '</name>\n')
                    xml_file.write('        <pose>Unspecified</pose>\n')
                    xml_file.write('        <truncated>0</truncated>\n')
                    xml_file.write('        <difficult>0</difficult>\n')
                    xml_file.write('        <bndbox>\n')
                    xml_file.write('            <xmin>' + str(xmin) + '</xmin>\n')
                    xml_file.write('            <ymin>' + str(ymin) + '</ymin>\n')
                    xml_file.write('            <xmax>' + str(xmax) + '</xmax>\n')
                    xml_file.write('            <ymax>' + str(ymax) + '</ymax>\n')
                    xml_file.write('        </bndbox>\n')
                    xml_file.write('    </object>\n')
            xml_file.write('</annotation>')
            File_2.close()


# In[ ]:




