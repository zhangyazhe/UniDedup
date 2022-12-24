#!/bin/bash

user="$USER"

node_num=26
node_name=node

filename=(~/patch.sh ~/copy-id.sh)
# cmd=

for((i=2;i<=$node_num;i++))
do
{
    if [[ $i -gt 0 && $i -lt 10 ]]
    then
        host=${node_name}0$i
    else
        host=${node_name}$i
    fi

    # for name in ${filename[@]}
    # do
    #     scp $name $user@$host:$name 
    # done
    
    ssh $user@$host "cd $HOME; ./patch.sh $i"
    # ssh $user@$host "ssh-keygen -t rsa"
} &
done

wait