import subprocess
import threading
from time import sleep

START = 1
NUM = 8

remote_path="/home/openec/Fast23/storage"

# for((i=1;i<=$node_num;i++));
# do
# {
#     i=$i
# 	if [[ $i -gt 0 && $i -lt 10 ]]
# 	then
# 		host=${node_name}0${i}
# 	else
# 		host=${node_name}$i
# 	fi

#     ssh $user@$host "source /etc/profile; cd $remote_path; ./start_ag.sh;"
# 	sleep 0.5
# }
# done
def format_node_name(node_id):
    return "node{:0>2}".format(node_id)

def start_agent(idx):
    ssh_cmd = "ssh openec@" + format_node_name(idx)
    cmd = ssh_cmd + " \"source /etc/profile; cd " + remote_path + "; ./start_ag.sh;\"" + " > log/agoutput" + str(idx) + ".txt"
    subprocess.call(cmd, shell=True)

if __name__ == "__main__":
    thread_list = []
    for i in range(NUM):
        thread_list.append(threading.Thread(target=start_agent, args=(i+START, )))
    
    cnt = 0
    for i in thread_list:
        print("------------------- " + format_node_name(cnt+START) + " start agent " + " -------------------")
        i.start()
        # sleep(2)
        print("------------------- " + format_node_name(cnt+START) + " start ok " + " -------------------")
        cnt += 1
    
    for i in thread_list:
        i.join()