#!/bin/bash

user="$USER"
node_num=16
node_name=node

home=/home/$user
remotePath=/$home/Fast23

# stop openec

echo ------------stop coordinator-------------
redis-cli flushall
killall OECCoordinator

for((i=1;i<=$node_num;i++));
do
{ 
	# if [[ $i -gt 3 && $i -lt 6 ]]
	# then
	# 	continue
	# fi
    if [[ $i -gt 0 && $i -lt 10 ]]
	then
		host=${node_name}0${i}
	else
		host=${node_name}$i
	fi

    echo ------------stop agent$i-------------
    ssh $user@$host "killall OECAgent; redis-cli flushall"
} 
done
# wait

# stop destor

for((i=1;i<=$node_num;i++));
do
{
    i=$i
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

    ssh $user@$host "$remotePath/destor/stop.sh"
} 
done
