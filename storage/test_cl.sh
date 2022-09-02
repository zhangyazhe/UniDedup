#!/bin/bash

user="$USER"

home=/home/$user
hadoop_home=$home/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0

sed -i 's/\/addresses.txt/\/cl_addresses.txt/' ./limit_bandwidth.sh 
sed -i 's/LRCSetting.xml/CLSetting.xml/' ./src/common/Config.hh

cmake . -DFS_TYPE:STRING=HDFS3

make

cp ./hdfs3-integration/conf/hdfs-site.xml $hadoop_home/etc/hadoop/

./autoSync.sh

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

./bw_limit_clean.sh
./bw_limit_rack.sh 800 80 26

./OECCoordinator