#!/bin/bash

user="$USER"
node_num=16
node_name=node

home=/home/$user
remotePath=/$home/Fast23
remoteConfPath=$remotePath/client/config
localPath=../client

hadoop_home=$home/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0
exec_files=(../client/client ../client/agent ../destor/proxy ../destor/destor ../client/cls.sh)
# conf_files=(pipeSetting.xml asSetting.xml aspSetting.xml convSetting.xml rpconvSetting.xml ppctSetting.xml smlzSetting.xml)

conf_files=($(ls ../client/config/))

# for i in ${conf_files[@]}
# do
# 	echo $i
# done

# for i in ${shell_files[@]}
# do
# 	echo $i
# done

ip_lists=(
	192.168.1.1
	192.168.1.2
	192.168.1.3
	192.168.1.4
	192.168.1.5
	192.168.1.6
	192.168.1.7
	192.168.1.8
	192.168.1.9
	192.168.1.10
	192.168.1.11
	192.168.1.12
	192.168.1.13
	192.168.1.14
	192.168.1.15
	192.168.1.16
	192.168.1.17
)

namenode_ip=${ip_lists[0]}

for((i=1;i<=$node_num;i++));
do
{
# 	if [[ $i -gt 3 && $i -lt 6 ]]
# 	then
# 		continue
# 	fi

	i=$i
	if [[ $i -gt 0 && $i -lt 10 ]]
	then
		host=${node_name}0${i}
	else
		host=${node_name}$i
	fi
	
	ip=${ip_lists[$i]}

	echo ---------autoSync $user@$host start------------

	for file in ${exec_files[@]}
	do
		rsync -a $localPath/$file $user@$host:$remotePath/client/$file
	done

	# for file in ${shell_files[@]}
	# do
	# 	rsync -a $localPath/$file $user@$host:$remotePath/$file
	# done

	# for j in ${!conf_files[@]}
	# do
	# 	file=${conf_files[$j]}
	# 	rsync -a $localPath/config/$file $user@$host:$remoteConfPath/$file
	# 	# ssh $user@$host "sudo sed -i \"s/<attribute><name>local.addr<\/name><value>$namenode_ip<\/value><\/attribute>/<attribute><name>local.addr<\/name><value>$ip<\/value><\/attribute>/\"" $remoteConfPath/$file
	# done

	echo ---------autoSync $user@$host finish------------
} &
done

wait