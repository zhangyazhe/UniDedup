#!/usr/bin/expect

set index [lindex $argv 0]
set node_num 32
set node_name node

for {set i 0} {$i <= $node_num} {incr i} {
    if {$i == $index} {
        continue
    }
    if {$i > 0 && $i < 10} {
        set host ${node_name}0$i
    } else {
        if {$i == 0} {
            set host master
        } else {
            set host ${node_name}$i
        }
    }

    spawn ssh-copy-id $host
    expect {
        "yes/no" { send "yes\r" }
        "password" { send "123456\r" }
    }
    expect "password" { send "123456\r" } 

    expect eof
}