#!/bin/bash

user="$USER"

hosts=(node01 node02 node03 node04 node05 node06 node07 node08 node09 node10)

home=/home/$user
hadoop_home=$home/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0

remote_path=/home/$user/openec/res/openec/

cmake . -DFS_TYPE:STRING=HDFS3

make

./autoSync.sh

cp ./hdfs3-integration/conf/hdfs-site.xml $hadoop_home/etc/hadoop/

./format.sh

if [ -f "entryStore" ];
then
    rm entryStore
fi

if [ -f "poolStore" ];
then
    rm poolStore
fi

redis-cli flushall

sudo service redis_6379 restart

./OECCoordinator &

sleep 2

for i in ${!hosts[@]}
do
{
    host=${hosts[$i]}

    ssh -t $user@$host "cd $remote_path; ./start_ag.sh;"
} &
done

wait