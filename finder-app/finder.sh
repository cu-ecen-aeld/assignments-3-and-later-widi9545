#!/bin/sh

filesdir=$1
searchstr=$2

if [ $# -eq 0 ]
then
    echo "Enter input parameters";
    exit 1 
fi

if ! [ -d "${filesdir}" ]
then
echo "This is not a directory path - enter a directory path";
exit 1
fi

fileReturn=`ls -l $filesdir/* | wc -l`
matchingLines=`grep -r $filesdir -e $searchstr | wc -l`

echo "The number of files are $fileReturn and the number of matching lines are $matchingLines"
