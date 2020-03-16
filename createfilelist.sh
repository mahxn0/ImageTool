# /usr/bin/env sh
DATA=/home/mahxn0/caffe/data/watch_data/val  #DATA为训练集的路径
echo "Create val.txt..."
rm -rf $DATA/val.txt
find $DATA -name First*.jpg | cut -d '/' -f8 | sed "s/$/ 0/">>$DATA/val.txt
find $DATA -name Second*.jpg | cut -d '/' -f8 | sed "s/$/ 1/">>$DATA/val.txt
find $DATA -name Third*.jpg | cut -d '/' -f8 | sed "s/$/ 2/">>$DATA/val.txt
find $DATA -name Forth*.jpg | cut -d '/' -f8 | sed "s/$/ 3/">>$DATA/val.txt
echo "Done.."

