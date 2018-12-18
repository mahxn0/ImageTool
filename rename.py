import os
import cv2
inputdir='/home/mahxn0/M_DeepLearning/models/research/deeplab/datasets/RoadMarking/label/'
outputdir='/home/mahxn0/M_DeepLearning/models/research/deeplab/datasets/RoadMarking/mask/'

filenames=os.listdir(inputdir)
for i in filenames:
    prename=i.split('_')[0]
    name=prename+'.png'
    inputname=os.path.join(inputdir,i)
    outputname=os.path.join(outputdir,name)
    print(inputname)
    print(outputname)
    image=cv2.imread(inputname)
    cv2.imwrite(outputname,image)
