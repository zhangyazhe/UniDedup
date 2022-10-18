#!/bin/bash

user="$USER"

node_num=$3
node_name=node

inner=$1
cross=$2

for((i=1;i<=$node_num;i++))
do
{
    i=$i
    if [[ $i -gt 0 && $i -lt 10 ]]
	then
		host=${node_name}0${i}
	else
		host=${node_name}$i
	fi

    node_index=$i
    # echo ------------set agent$node_index bandwidth-------------
    ssh $user@$host "sudo /home/openec/Fast23/storage/limit_bandwidth.sh rack $inner $cross $node_index"
} &
done

wait 