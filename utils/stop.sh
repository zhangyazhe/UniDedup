#!/bin/bash

user="$USER"
node_num=8
node_name=node

home=/home/$user
remotePath=/$home/Fast23
remoteConfPath=$remotePath/client/conf
localPath=../client

for((i=0;i<=$node_num;i++));
do
{
    if [[ $i -eq 5 ]]
    then
        continue
    fi

    if [[ $i -gt 0 && $i -lt 10 ]]
	then
		host=${node_name}0${i}
	else
        if [[ $i -eq 0 ]]
        then
            host=master
        else
		    host=${node_name}$i
        fi
	fi

    ssh $user@$host "$remotePath/destor/stop.sh"
} &
done

wait