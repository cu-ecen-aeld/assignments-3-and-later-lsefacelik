#!/bin/bash

if [ $# != 2 ]
then    
    echo "Invalid number of arguments."
    exit 1
fi

WRITEFILE=$1
WRITESTR=$2

echo $WRITESTR > $WRITEFILE

if [ ! -f $WRITEFILE ]
then
    echo "File could not be created."
    exit 1
fi

exit 0
