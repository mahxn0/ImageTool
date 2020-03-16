import sys
import numpy as np
from PIL import Image
import time
import cv2
import os

inputDir='/home/mahxn0/M_DeepLearning/models/research/deeplab/datasets/RoadMarking/rect_mask'

class1 = 0
class2 = 0
class3 = 0
class4 = 0
class5 = 0
class6 = 0
class7 = 0
class8 = 0
class9 = 0
class10 = 0
class11 = 0
class12 = 0
class13=0
width = 0
height = 0
for root, dirs, files in os.walk(inputDir):
    for file in files:
        fname = inputDir + '/' +file
        if fname[-3:] == 'png':
            #fname = 'test.png'
            im = Image.open(fname)
            pix = im.load()
            width = im.size[0]
            height = im.size[1]
            #print width,height
            for x in range(width):
                for y in range(height):
                    r = pix[x,y]
                    if r == 0:
                        class1 += 1
                    elif r ==1:
                        class2 += 1
                    elif r == 2:
                        class3 += 1
                    elif r == 3:
                        class4 += 1
                    elif r ==4:
                        class5 += 1
                    elif r == 5:
                        class6 += 1
                    if r == 6:
                        class7 += 1
                    elif r ==7:
                        class8 += 1
                    elif r == 8:
                        class9 += 1
                    elif r == 9:
                        class10 += 1
                    elif r ==10:
                        class11 += 1
                    elif r == 11:
                        class12 += 1
                    elif r==12:
                        class13+=1


print class1,class2,class3,class4,class5,class6,class7,class8,class9,class10,class11,class12,class13
