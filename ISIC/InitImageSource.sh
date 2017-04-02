#!/bin/bash

if [ ! $# == 6 ]; then

echo "Usage : $0 [Offset] [ImageNum] [ImageSourcePath] [GreyImageOutputPath] [NormalizeMatrixPath] [CompressedMatrixPath]"
echo "e.g. : $0 1 4664 /dataset/ISIC_JPG /dataset/ISIC_Grey /dataset/ISIC_Mat /dataset/ISIC_CS"

exit

fi

Offset=$1
ImageNum=$2
ImageSourcePath=$3
GreyImageOutputPath=$4
NormalizeMatrixPath=$5
CompressedMatrixPath=$6

for ((i=Offset;i<ImageNum;i++))
do

    echo "./Normalize_Image ${ImageSourcePath}/${i}.jpg ${GreyImageOutputPath}/g_${i}.jpg ${NormalizeMatrixPath}/${i}"
    ./Normalize_Image ${ImageSourcePath}/${i}.jpg ${GreyImageOutputPath}/g_${i}.jpg ${NormalizeMatrixPath}/${i}

    echo "./Compress_Mat ${NormalizeMatrixPath}/${i} ${CompressedMatrixPath}/${i}"
    ./Compress_Mat ${NormalizeMatrixPath}/${i} ${CompressedMatrixPath}/${i} 

done

