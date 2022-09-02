#!/bin/bash

user="$USER"

bandwidth=$1

node_num=32
node_name=node

# echo ------------clean bandwidth limited-------------

for((i=1;i<=$node_num;i++))
do
{
    if [[ $i -gt 0 && $i -lt 10 ]]
    then
        host=${node_name}0$i
    else
        host=${node_name}$i
    fi

    # echo ------------limit_bandwidth agent$i-------------
    ssh $user@$host "sudo /home/openec/openec/res/openec/limit_bandwidth.sh all $bandwidth $i"
} &
done

wait 