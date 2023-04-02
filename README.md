# Unidedup

UniDedup is a distributed deduplication management system. With UniDedup, you can easily build a standalone deduplication system into a distributed deduplication system. UniDedup will help you to manage deduplication clusters.UniDedup has the following features:
1. each deduplication node performs deduplication independently, UniDedup is mainly responsible for metadata management and data routing. UniDedup can manage heterogeneous nodes or different deduplication systems at the same time.
2. encapsulate several consecutive data blocks into groups, which is the basic unit of data management by UniDedup. The introduction of groups can reduce the metadata used for data routing, and at the same time can preserve the localization of data, thus improving data recovery performance.
3. Using the similarity detection method, the feature values of each group are extracted and used as one of the main references when routing, and similar groups will be routed to the same deduplication node for deduplication.
4. UniDedup is able to obtain a deduplication rate very similar to that of a single node. In a four-node distributed deduplication test on rdb, gcc, linux, and webs datasets, the deduplication rate drops only less than 1.5% compared to the single machine deduplication rate.
5. Support load balancing.
   
## How to use UniDedup
Take the example of building a cluster with four nodes (one master node and three datanode nodes).

### Config
- Prepare four servers. The host names are: master, node01, node02, and ndoe03. ubuntu18.04 is running on each node. after entering ubuntu, create a user named openec on each node, and all subsequent operations will be performed under that user.
- Install vim, net-tools, and openssh-server on each node.
- Configure each node for password-free login.
- Configure each node to sudo-free.
- Modify the host name of each node to `master` `node01` `node02` `node03`.
- Modify `/etc/hosts` of master node, add ip of master, node01, node02 and node03
- Get the source code in each node and rename the UniDedup folder to Fast23.
- Execute `bash dependency/setup.sh`.
- Install redis 6.2.8, the installation package is in the `dependency/` folder. The local redis-server is started using redis.conf in `dependency/res/myredis`.
- Install redis++ on each node with a link to the code repository at https://github.com/sewenew/redis-plus-plus.git.
- In each datanode, copy everything in `~/.bashrc` from the first uncommented `export` to the end of `/etc/profile`.
- For all nodes, go to the `utils/` directory and execute `. /destor_setup.sh` to configure the destor environment.
- Build node01, node02 and node03 into a redis cluster, all using port 6380.
  
### Compile
- Compile destor. Go to the `destor/` directory and execute
  ```
  ./configure
  autoreconf -vfi
  make
  ```
- Compile client. Go to the `client/` directory and execute
  ```
  ./configure
  make
  ```
  
### Config files
- Config files of destor: `destor/destor.config`
- Config files of client：`client/config/`
  
### Execute
1. Execute the `make` command in the `client/` directory of the master node, and then execute `utils/auto_sync.sh` to distribute the compiled executable to other nodes.
2. In the `destor/` directory of each node, execute `. /rebuild.sh` command in each node's destor directory.
3. In the `utils/` directory of the master node, execute `. /start_destor.sh` command.
4. On any datanode, go to the `client/` directory and execute `. /agent command`, and then execute `. /client command` to read and write.
5. The above script should be modified before use, the node_num in the script should be changed to the number of data nodes, and the ip address should be changed to the ip of master node or datanode.
