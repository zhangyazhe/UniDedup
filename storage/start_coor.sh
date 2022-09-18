#!/bin/bash

user="$USER"

home=/home/$user
hadoop_home=$home/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0

# make clean

cmake . -DFS_TYPE:STRING=HDFS3

make

# ./autoSync.sh

# cp ./hdfs3-integration/conf/* $hadoop_home/etc/hadoop/

# ./format.sh

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

./OECCoordinator