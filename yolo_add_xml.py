#-*- coding=utf-8 -*-
from ctypes import *
import math
import random
import time
import cv2
import numpy as np
import re
import os
import sys
from xml.etree.ElementTree import ElementTree,Element


def sample(probs):
    s = sum(probs)
    probs = [a/s for a in probs]
    r = random.uniform(0, 1)
    for i in range(len(probs)):
        r = r - probs[i]
        if r <= 0:
            return i
    return len(probs)-1

# def c_array(ctype, values):
#     arr = (ctype*len(values))()
#     arr[:] = values
#     return arr

def c_array(ctype, values):
    return (ctype * len(values))(*values)

class BOX(Structure):
    _fields_ = [("x", c_float),
                ("y", c_float),
                ("w", c_float),
                ("h", c_float)]

class DETECTION(Structure):
    _fields_ = [("bbox", BOX),
                ("classes", c_int),
                ("prob", POINTER(c_float)),
                ("mask", POINTER(c_float)),
                ("objectness", c_float),
                ("sort_class", c_int)]


class IMAGE(Structure):
    _fields_ = [("w", c_int),
                ("h", c_int),
                ("c", c_int),
                ("data", POINTER(c_float))]

class METADATA(Structure):
    _fields_ = [("classes", c_int),
                ("names", POINTER(c_char_p))]


#lib = CDLL("/home/pjreddie/documents/darknet/libdarknet.so", RTLD_GLOBAL)
lib = CDLL("/home/mahxn0/darknet/darknet.so", RTLD_GLOBAL)
lib.network_width.argtypes = [c_void_p]
lib.network_width.restype = c_int
lib.network_height.argtypes = [c_void_p]
lib.network_height.restype = c_int

predict = lib.network_predict
predict.argtypes = [c_void_p, POINTER(c_float)]
predict.restype = POINTER(c_float)

set_gpu = lib.cuda_set_device
set_gpu.argtypes = [c_int]

make_image = lib.make_image
make_image.argtypes = [c_int, c_int, c_int]
make_image.restype = IMAGE

get_network_boxes = lib.get_network_boxes
get_network_boxes.argtypes = [c_void_p, c_int, c_int, c_float, c_float, POINTER(c_int), c_int, POINTER(c_int),c_int]
get_network_boxes.restype = POINTER(DETECTION)

make_network_boxes = lib.make_network_boxes
make_network_boxes.argtypes = [c_void_p]
make_network_boxes.restype = POINTER(DETECTION)

free_detections = lib.free_detections
free_detections.argtypes = [POINTER(DETECTION), c_int]

free_ptrs = lib.free_ptrs
free_ptrs.argtypes = [POINTER(c_void_p), c_int]

network_predict = lib.network_predict
network_predict.argtypes = [c_void_p, POINTER(c_float)]

reset_rnn = lib.reset_rnn
reset_rnn.argtypes = [c_void_p]

load_net = lib.load_network
load_net.argtypes = [c_char_p, c_char_p, c_int]
load_net.restype = c_void_p

do_nms_obj = lib.do_nms_obj
do_nms_obj.argtypes = [POINTER(DETECTION), c_int, c_int, c_float]

do_nms_sort = lib.do_nms_sort
do_nms_sort.argtypes = [POINTER(DETECTION), c_int, c_int, c_float]

free_image = lib.free_image
free_image.argtypes = [IMAGE]

letterbox_image = lib.letterbox_image
letterbox_image.argtypes = [IMAGE, c_int, c_int]
letterbox_image.restype = IMAGE

load_meta = lib.get_metadata
lib.get_metadata.argtypes = [c_char_p]
lib.get_metadata.restype = METADATA

load_image = lib.load_image_color
load_image.argtypes = [c_char_p, c_int, c_int]
load_image.restype = IMAGE

rgbgr_image = lib.rgbgr_image
rgbgr_image.argtypes = [IMAGE]

predict_image = lib.network_predict_image
predict_image.argtypes = [c_void_p, IMAGE]
predict_image.restype = POINTER(c_float)

ndarray_image = lib.ndarray_to_image
ndarray_image.argtypes = [POINTER(c_ubyte), POINTER(c_long), POINTER(c_long)]
ndarray_image.restype = IMAGE

#helmet model
#net = load_net("/home/mahxn0/darknet/yolo_helmet/helmet.cfg", "/home/mahxn0/darknet/yolo_helmet/helmet_final.weights", 0)
#meta = load_meta("/home/mahxn0/darknet/yolo_helmet/helmet.data")
#voc model
net = load_net("/home/mahxn0/darknet/cfg/yolov3.cfg", "/home/mahxn0/darknet/yolov3.weights", 0)
meta = load_meta("/home/mahxn0/darknet/cfg/coco.data")

#tab xml
def prettyXml(element, indent, newline, level=0):
    if element:
        if element.text == None or element.text.isspace():
            element.text = newline + indent * (level + 1)
        else:
            element.text = newline + indent * (level + 1) + element.text.strip() + newline + indent * (level + 1)
    temp = list(element)
    for subelement in temp:
        if temp.index(subelement) < (len(temp) - 1): 
            subelement.tail = newline + indent * (level + 1)
        else:
            subelement.tail = newline + indent * level
        prettyXml(subelement, indent, newline, level=level + 1)

class yolo_helmet(object):
    def __init__(self):
        pass
    def detect_pic(self, image, thresh=0.1, hier_thresh=.5, nms=.45):
        im = self.nparray_to_image(image)
        num = c_int(0)
        pnum = pointer(num)
        predict_image(net, im)
        dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, None, 0, pnum,0)
        num = pnum[0]
        if (nms): do_nms_obj(dets, num, meta.classes, nms)
        res = []
        for j in range(num):
            for i in range(meta.classes):
                if dets[j].prob[i] > 0:
                    b = dets[j].bbox
                    left=(b.x-b.w/2)
                    right=(b.x+b.w/2)
                    top=(b.y-b.h/2)
                    bot=(b.y+b.h/2)
                    if left < 0:
                            left = 0
                    if right > im.w-1:
                            right = im.w-1
                    if top < 0:
                            top = 0
                    if bot > im.h-1:
                            bot = im.h-1
                    if meta.names[i]=='person':
                        res.append((meta.names[i], dets[j].prob[i],left,top,right,bot))
        res = sorted(res, key=lambda x: -x[1])
        free_image(im) #not sure if this will cause a memory leak.
        free_detections(dets, num)
        return res

    def detect(self, imagename, thresh=.4, hier_thresh=.5, nms=.45):
        t0=time.time()
    	#im=self.array_to_image(image)
        #rgbgr_image(im)
        image=cv2.imread(imagename)
        im = self.nparray_to_image(image)
        t1=time.time()
        num = c_int(0)
        pnum = pointer(num)
        predict_image(net, im)
        dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, None, 0, pnum,0)
        num = pnum[0]
        if (nms): do_nms_obj(dets, num, meta.classes, nms)
        res = []
        for j in range(num):
            for i in range(meta.classes):
                if dets[j].prob[i] > 0:
                    b = dets[j].bbox
                    left=(b.x-b.w/2)-5
                    right=(b.x+b.w/2)+5
                    top=(b.y-b.h/2)-5
                    bot=(b.y+b.h/2)+5
                    if left < 0:
                            left = 0
                    if right > im.w-1:
                            right = im.w-1
                    if top < 0:
                            top = 0
                    if bot > im.h-1:
                            bot = im.h-1
                    if meta.names[i]=='person':
                        res.append((meta.names[i], dets[j].prob[i],left,top,right,bot))
        res = sorted(res, key=lambda x: -x[1])
        free_image(im) #not sure if this will cause a memory leak.
        free_detections(dets, num)
        t2=time.time()
        print("detect take %d s",t2-t0);
        print("array_to_image take %d s",t1-t0)
        return res
    def array_to_image(self,arr):
         arr = arr.transpose(2,0,1)
         c = arr.shape[0]
         h = arr.shape[1]
         w = arr.shape[2]
         arr = (arr/255.0).flatten()
         data = c_array(c_float, arr)
         im = IMAGE(w,h,c,data)
         return im

    def nparray_to_image(self,img):
        data = img.ctypes.data_as(POINTER(c_ubyte))
        image = ndarray_image(data, img.ctypes.shape, img.ctypes.strides)
        return image

    def getXY(self,i):
        return int(i)
if __name__ == "__main__":
    imgDir = '/media/mahxn0/DATA/dataset/helmet_hand_data/hand/train_img'
    filenames = os.listdir(imgDir)
    r=yolo_helmet()
    for name in filenames:
        imgName = os.path.join(imgDir,name)
        if 'jpg' in name:
            print(imgName)
            img = cv2.imread(imgName)
            xmlName = os.path.join(imgDir,name[:-3]+'xml')
            rets = r.detect(imgName)
            tree=ElementTree()
            tree.parse(xmlName)
            root=tree.getroot()
            if len(rets)>0:
                for obj in rets:
                    element=Element('object')
                    name=Element('name')
                    pose=Element('pose')
                    truncated=Element('truncated')
                    Difficult=Element('Difficult')
                    pose.text='Unspecified'
                    truncated.text='0'
                    Difficult.text='0'
                    bndbox=Element('bndbox')
                    name.text = str(obj[0])
                    xmin=Element('xmin')
                    xmin.text = str(int(obj[2]))
                    ymin=Element('ymin')
                    ymin.text = str(int(obj[3]))
                    xmax=Element('xmax')
                    xmax.text = str(int(obj[4]))
                    ymax=Element('ymax')
                    ymax.text = str(int(obj[5]))
                    bndbox.append(xmin)
                    bndbox.append(ymin)
                    bndbox.append(xmax)
                    bndbox.append(ymax)
                    element.append(name)
                    element.append(pose)
                    element.append(truncated)
                    element.append(Difficult)
                    element.append(bndbox)
                    root.append(element)
            prettyXml(root, '\t', '\n')
            tree.write(xmlName,encoding='utf-8',xml_declaration=True)
                    
