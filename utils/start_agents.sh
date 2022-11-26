#!/bin/bash

user="$USER"
node_num=3
node_name=node

home=/home/$user
path=/$home/Fast23

# start openec
for((i=1;i<=$node_num;i++))
do
{
    # if [[ $i -gt 3 && $i -lt 6 ]]
    # then
    #     continue
    # fi
    i=$i
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
    if [ $host = "master" ]
    then
        cd $path/storage
        ./OECAgent
    else
        ssh $user@$host "source /etc/profile; cd $path/storage; ./OECAgent" > log/log$i.txt
    fi
} &
done
wait
