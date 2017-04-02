#!/bin/bash

if [ ! $# == 3 ]; then

echo "Usage : $0 [ImageCSPath] [ImageRecoverOutputPath] [RecoverIDFilePath]"
echo "e.g. : $0 /dataset/ISIC_CS /dataset/ISIC_Recover IDS"

exit

fi

ImageCSPath=$1
ImageRecoverOutputPath=$2
RecoverIDFilePath=$3

while read LINE
do

    echo "nohup ./Recover_Mat $ImageCSPath/$LINE $ImageRecoverOutputPath/$LINE &"
    nohup ./Recover_Mat $ImageCSPath/$LINE $ImageRecoverOutputPath/$LINE &

done < $RecoverIDFilePath
