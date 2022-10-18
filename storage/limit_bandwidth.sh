#!/bin/bash 
dev_names=(
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
    eth0
)

IFB=ifb0

USER=openec
HOME=/home/$USER
OPENEC=$HOME/Fast23/storage

usage() {
    # echo "usage:  all bandwidth<mbit/s>"
    # echo "        rack bandwidth<mbit/s> [node_index]"
    # echo "        clean"
    exit
}

clean() {
    # del qdisc and filter   #  > /dev/null 2>&1
    if [[ $1 == 1 ]]
    then 
        tc filter del dev ${dev_name} > /dev/null 2>&1
        # echo "clean filter"
    fi
    tc qdisc del dev ${dev_name} root > /dev/null 2>&1
    # echo "clean qdisc"
    tc qdisc del dev ${dev_name} root    2> /dev/null > /dev/null
    tc qdisc del dev ${dev_name} ingress 2> /dev/null > /dev/null
    tc qdisc del dev $IFB   root    2> /dev/null > /dev/null
    tc qdisc del dev $IFB   ingress 2> /dev/null > /dev/null
}

get_index_from_hostname() {
    hostname=$(cat /etc/hostname)
    node_index=${hostname: 4}
    # echo "extract "$node_index
    expr $node_index "+" 10 &> /dev/null
    if [[ $? -ne 0 ]]
    then
        # echo "Need to set hostname: node{%2d}"
        # echo "Or you can provide node_index"
        # echo ""
        usage
    fi
    if [[ ${node_index:0:1} = "0" ]]
    then
        node_index=${node_index:1}
    fi

}

get_ip_to_set() {
    mapfile config < $OPENEC/conf/cl_addresses.txt

    N=${config[0]}
    k=${config[1]}
    m=${config[2]}

    let node=node_index%m
    if [[ $node -eq 0 ]]
    then 
        let start_index=node_index-m
    else
        let start_index=node_index-node_index%m
    fi
    let end_index=start_index+m-1

    let set_num=end_index-start_index
    ip_to_set=(0)
    for (( i=$start_index, j=0; i <= $end_index; i++ ))
    do
        if [[ $i -eq `expr $node_index-1` ]]
        then
            continue
        fi
        let index=i+5
        cur_ip=$(echo ${config[$index]} | tr '\r' ' ' | tr '\n' ' ' )    
        ip_to_set[j]=$cur_ip
        let "j++"
    done
}

set_rack_bandwidth() {
    # upload
    # create root qdisc 
    tc qdisc add dev ${dev_name} root handle 1: htb default 10

    # create main class and bind
    tc class add dev ${dev_name} parent 1: classid 1:1 htb rate ${inbandwidth}Mbit ceil 12000Mbit burst 20mb
    # create sub class, for inner-rack and cross-rack
    tc class add dev ${dev_name} parent 1:1 classid 1:10 htb rate ${bandwidth}Mbit burst 20mb
    tc class add dev ${dev_name} parent 1:1 classid 1:20 htb rate ${inbandwidth}Mbit ceil 12000Mbit burst 20mb

    # use Fairness Queueing
    tc qdisc add dev ${dev_name} parent 1:10 handle 10: sfq perturb 10  
    tc qdisc add dev ${dev_name} parent 1:20 handle 20: sfq perturb 10

    # download
    modprobe ifb numifbs=1
    ip link set dev $IFB up

    # Redirect ingress (incoming) to egress ifb0
    tc qdisc add dev ${dev_name} handle ffff: ingress
    tc filter add dev ${dev_name} parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev $IFB

    # Add class and rules for virtual
    tc qdisc add dev $IFB root handle 2: htb default 10
    tc class add dev $IFB parent 2: classid 2:1 htb rate ${inbandwidth}Mbit ceil 12000Mbit burst 20mb

    # create sub class, for inner-rack and cross-rack
    tc class add dev $IFB parent 2:1 classid 2:10 htb rate ${bandwidth}Mbit burst 20mb
    tc class add dev $IFB parent 2:1 classid 2:20 htb rate ${inbandwidth}Mbit ceil 12000Mbit burst 20mb

    # use Fairness Queueing
    tc qdisc add dev $IFB parent 2:10 handle 10: sfq perturb 10
    tc qdisc add dev $IFB parent 2:20 handle 20: sfq perturb 10

    # Add filter to rule for IP address
    
    # create filters
    # inner-rack case, with higher priority, full rate
    get_ip_to_set
    for (( i=0; i < $set_num; i++ ))
    do
        cur_ip=${ip_to_set[i]}
        tc filter add dev ${dev_name} protocol ip parent 1:0 prio 1 u32 match ip dst ${cur_ip} flowid 1:20
        tc filter add dev $IFB protocol ip parent 2: prio 1 u32 match ip src ${cur_ip} flowid 2:20
    done
}

# invalid call
if [[ $# == 0 || $# -gt 4 || $# == 1  && $1 != "clean" ]]
then
    usage
fi

# main process
if [[ $1 == "all" ]]
then
    bandwidth=$2
    node_index=$3
    # echo $node_index
    dev_name=${dev_names[node_index]}
    # echo $dev_name
    tc qdisc del dev ${dev_name} root > /dev/null 2>&1
    # clean 0
    # echo "set bandwidth "$bandwidth" mbit/s"
    tc qdisc add dev ${dev_name} root tbf rate ${bandwidth}mbit burst 10mb latency 50ms
    exit
elif [[ $1 == "rack" ]]
then
    if [[ $# == 4 ]]
    then
        node_index=$4
        dev_name=${dev_names[node_index]}
    else
        get_index_from_hostname
    fi
    inbandwidth=$2
    bandwidth=$3
    # echo "node_index = "$node_index
    clean 1
    # echo "set cross-rack bandwidth "$bandwidth" mbit/s"
    set_rack_bandwidth
    # get_ip_to_set
    # for i in ${ip_to_set[@]}
    # do
    #     echo $i
    # done
    exit
elif [[ $1 == "clean" ]]
then
    # clean previous rules
    node_index=$2
    dev_name=${dev_names[node_index]}
    clean 1
    exit
else
    usage
fi
