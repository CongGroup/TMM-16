#!/bin/bash

if [ ! $# == 8 ]; then

echo "Usage : ./TestThroughput.sh [OP] [LOAD] [LSH] [INDEX_NUM] [INDEX_B] [LOOP] [DURATION] [LSHL]"
echo "e.g. : ./TestThroughput.sh Q 0.9 1298600014 1000000000 50 16 10 10"
echo "e.g. : ./TestThroughput.sh Q 0.9 1000000 1000000 50 16 10 10"
exit

fi

CMD="BuildThroughput"
if [ "$1"x == "Q"x ]; then

    CMD="QueryThroughput"

fi

LOAD=$2
LSH_NUM=$3
INDEX_NUM=$4
INDEX_B=$5
LOOP=$6
DURATION=$7
LSHL=$8

BEGTIME=`date +%s -d "30 seconds"`

get_char()
{
        SAVEDSTTY=`stty -g`
        stty -echo
        stty raw
        dd if=/dev/tty bs=1 count=1 2> /dev/null
        stty -raw
        stty echo
        stty $SAVEDSTTY
}

rm -f OutTestThroughput

for i in $(seq 1 ${LOOP})
do

echo "./$CMD $BEGTIME $DURATION $LSH_NUM $i $INDEX_NUM $LOAD $INDEX_B $LSHL"
./$CMD $BEGTIME $DURATION $LSH_NUM $i $INDEX_NUM $LOAD $INDEX_B $LSHL >> OutTestThroughput &

done

echo "Press any key to continue..."
char=`get_char`

awk 'BEGIN{total=0}{total+=$1}END{print total}' OutTestThroughput



