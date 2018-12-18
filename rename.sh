#!/bin/bash
# Author: Gihan De Silva @  gihansblog.com
# rename script
# rename.sh
clear
x=0
for i in `ls *.bmp`
do
x=`expr $x + 1`
convert $i $x.jpg
done


echo “rename done!”
