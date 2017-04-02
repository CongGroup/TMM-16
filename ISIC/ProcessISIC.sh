#!/bin/bash

if [ ! $# == 3 ]; then

echo "Usage : $0 [Offset] [EachNum] [MaxNum]"
echo "e.g. : $0 1 300 4665"

exit

fi

Offset=$1
EachNum=$2
MaxNum=$3

for ((i=Offset;i<=MaxNum;i+=EachNum))
do

    ((EndNum=i+EachNum))

    if ((EndNum>MaxNum)); then
        ((EndNum=MaxNum))
    fi

    echo "./InitImageSource.sh ${i} ${EndNum} /dataset/ISIC_JPG /dataset/ISIC_Grey /dataset/ISIC_Mat /dataset/ISIC_CS_05"

    nohup ./InitImageSource.sh ${i} ${EndNum} /dataset/ISIC_JPG /dataset/ISIC_Grey /dataset/ISIC_Mat /dataset/ISIC_CS_05 &
  
done

