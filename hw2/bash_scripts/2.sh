#!/bin/bash

array=($(shuf -i 0-9 -n 10))

for e in ${array[@]}; do
    printf "$e "
done

echo

