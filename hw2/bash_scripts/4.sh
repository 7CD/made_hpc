#!/bin/bash

file="Linux"

if [ -e $file ]; then
    echo "course"
else 
    echo "very easy"
    echo course is easy > Linux
fi 
