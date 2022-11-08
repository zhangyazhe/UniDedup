# Fast23
## 开发注意事项
开发时，先切换到自己的分支，以避免出现conflict。开发完成后，由所有开发者进行code review，没问题后再合并到主分支。
## 安装
0. 以搭建一个拥有三个节点的集群为例。
1. 首先准备三台服务器，或者创建三个虚拟机。主机名分别命名为：master、node01、node02。节点或虚拟机中运行ubuntu18.04。进入ubuntu后，各个节点均创建一个名为openec的用户，之后的操作均在该用户下进行。
2. 在每个节点安装vim、net-tools、openssh-server、make、gcc。
3. 配置master免密登陆自己，以及master免密登陆其他node。
4. 配置sudo免密
5. 修改master节点的/etc/hosts，添加master、node01、node02的ip
6. 执行dependency目录下的setup.sh，安装依赖
7. run destor_setup.sh in utils
8. 使用三个节点搭建redis集群