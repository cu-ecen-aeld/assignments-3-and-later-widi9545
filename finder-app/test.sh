#!/usr/bin/bash

writefile=$1
writestr=$2

if [ $# -eq 0 ]
then
    echo "Enter input parameters";
    exit 1 
fi

if [ -z $2 ]
then
    echo "Enter an input string";
    exit 1 
fi

parent=$(dirname $writefile)

if ! [ -d "${parent}" ]
then
mkdir $parent
fi
