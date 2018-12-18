from PIL import Image
import os


input_path="/home/mahxn0/M_DeepLearning/models/research/deeplab/datasets/RoadMarking/rect_mask/"
output_path="/home/mahxn0/M_DeepLearning/models/research/deeplab/datasets/RoadMarking/rect_mask0/"
filenames=os.listdir(input_path)

for i in filenames:
    input_filename=os.path.join(input_path,i)
    output_filename=os.path.join(output_path,i)
    I = Image.open(input_filename)
    L = I.convert('L')
    L.save(output_filename)

