#!/bin/bash

USER="$USER"
HOME=/home/$USER
NODE_NAME=node
NODE_NUM=8
HADOOP_HOME=$HOME/hadoop-3.0.0-src/hadoop-dist/target/hadoop-3.0.0
HDFS_TMP_DIR=$HADOOP_HOME/data

stop-dfs.sh

for((i=1;i<=$NODE_NUM;i++));
do
{
    if [[ $i -gt 0 && $i -lt 10 ]]
	then
		host=${NODE_NAME}0${i}
	else
		host=${NODE_NAME}$i
	fi

    echo ---------format $user@$host start------------

    ssh $USER@$host "if [[ -d $HDFS_TMP_DIR ]]; then cd $HDFS_TMP_DIR; rm -rf ./*; fi;"

    echo ---------format $user@$host finish------------
} &
done

wait

hdfs namenode -format

start-dfs.sh
