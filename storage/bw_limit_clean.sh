#!/bin/bash

user=openec

node_num=12
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

    # echo ------------clean agent$i-------------
    ssh $user@$host "sudo /home/openec/Fast23/storage/limit_bandwidth.sh clean $i"
} &
done

wait