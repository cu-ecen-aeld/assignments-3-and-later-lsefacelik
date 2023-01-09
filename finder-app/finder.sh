#!/bin/sh

if [ $# != 2 ]
then    
    echo "Invalid number of arguments."
    exit 1
fi

FILESDIR=$1
SEARCHSTR=$2

if [ ! -d $FILESDIR ]
then
    echo "Invalid directory path."
    exit 1
fi

FILES=$(grep -rcl "${SEARCHSTR}" "${FILESDIR}"| wc -l)

LINES=$(grep -r "${SEARCHSTR}" "${FILESDIR}" | wc -l)

echo "The number of files are $FILES and the number of matching lines are $LINES"

exit 0
