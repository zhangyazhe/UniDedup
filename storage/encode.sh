
#!/bin/bash

pcount=$#

if [[ $pcount != 7 ]];
then
    echo "parameter number error!"
    echo "-------------usage------------"
    echo "./encode stripe_num k block_path saveas ec_pool_id off_or_on block_size"
    echo "------------------------------"
    exit
fi

stripe_num=$1
k=$2
block_path=$3
saveas=$4
ec_pool_id=$5
ec_policy=$6
block_size=$7

# tesfile should be at the same path with encode.sh and the size of testfile should be the same as blocksize

for((i=0;i<$stripe_num;i++))
do
    for((j=0;j<$k;j++))
    do
        block_id=$(($i*$k+$j))
        # echo $saveas$block_id
        sleep 1
        ./OECClient write $block_path $saveas$block_id $ec_pool_id $ec_policy $block_size
    done
    sleep 5
done
