#!/bin/bash

user="$USER"
node_num=8
node_name=node

home=/home/$user
remotePath=$home/Fast23/storage
remoteConfPath=$remotePath/conf
localPath=./

hadoop_home=$home/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0
exec_files=(OECClient OECAgent)
# conf_files=(pipeSetting.xml asSetting.xml aspSetting.xml convSetting.xml rpconvSetting.xml ppctSetting.xml smlzSetting.xml)

conf_files=($(ls ./conf/))
shell_files=($(ls *.sh))

# for i in ${conf_files[@]}
# do
# 	echo $i
# done

# for i in ${shell_files[@]}
# do
# 	echo $i
# done

hdfs_files=(hdfs-site.xml core-site.xml hadoop-env.sh workers)

ip_lists=(
	192.168.235.179
	192.168.235.180
	192.168.235.181
	192.168.235.182
	192.168.235.183
	192.168.235.184
	192.168.235.185
	192.168.235.186
	192.168.235.187
)

namenode_ip=${ip_lists[0]}

for((i=1;i<=$node_num;i++));
do
{
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
		rsync -a $localPath/$file $user@$host:$remotePath/$file
	done

	for file in ${shell_files[@]}
	do
		rsync -a $localPath/$file $user@$host:$remotePath/$file
	done

	for j in ${!conf_files[@]}
	do
		file=${conf_files[$j]}
		rsync -a $localPath/conf/$file $user@$host:$remoteConfPath/$file
		ssh $user@$host "sudo sed -i \"s/<attribute><name>local.addr<\/name><value>$namenode_ip<\/value><\/attribute>/<attribute><name>local.addr<\/name><value>$ip<\/value><\/attribute>/\"" $remoteConfPath/$file
	done


	for hdfs_file in ${hdfs_files[@]}
	do
		rsync -a $localPath/hdfs3-integration/conf/$hdfs_file $user@$host:$hadoop_home/etc/hadoop/
		ssh $user@$host "sudo sed -i \"s/<property><name>oec.local.addr<\/name><value>192.168.235.159<\/value><\/property>/<property><name>oec.local.addr<\/name><value>$ip<\/value><\/property>/\"" $hadoop_home/etc/hadoop/$hdfs_file
	done

	echo ---------autoSync $user@$host finish------------
} &
done

wait