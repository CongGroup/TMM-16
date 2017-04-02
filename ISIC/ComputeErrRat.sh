#!/bin/bash

if [ ! $# == 3 ]; then

echo "Usage : $0 [InputImageMatrix] [InputRecoverMatrix] [IDFilePath]"
echo "e.g. : $0 /dataset/ISIC_Mat /dataset/ISIC_Recover IDS"

exit

fi

InputImageMatrix=$1
InputRecoverMatrix=$2
RecoverIDFilePath=$3

while read LINE
do

    ./Compute_ValueErr $InputImageMatrix $InputRecoverMatrix $LINE

done < $RecoverIDFilePath
