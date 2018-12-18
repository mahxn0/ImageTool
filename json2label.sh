#!/bin/bash
s1="/home/mahxn0/M_DeepLearning/models/research/object_detection/mask_rcnn_test/train_data/json/rgb_"
s2="-7.json"
for((i=0;i<360;i++))
do 
s3=${i}
labelme_json_to_dataset ${s1}${s3}${s2}
done


