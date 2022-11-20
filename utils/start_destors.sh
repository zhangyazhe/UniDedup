#!/bin/bash

user="$USER"
node_num=2
node_name=node

home=/home/$user
path=/$home/Fast23

for((i=1;i<=$node_num;i++));
do
{
    # if [[ $i -gt 3 && $i -lt 6 ]]
    # then
    #     continue
    # fi
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

    ssh $user@$host "source /etc/profile; cd $path/destor; ./proxy"
} &
done

# wait