# 配置openEC

### 安装依赖

```shell
sudo apt-get install -y g++ libssl-dev autoconf libtool yasm libz-dev gcc-multilib g++-multilib openssh-server
```

### 编写~/.bashrc

```shell
# cmake
export LD_LIBRARY_PATH=/usr/local/ssl/include/openssl:/usr/lib:/usr/local/lib:/usr/lib/pkgconfig:/usr/local/include/wx-2.8/wx:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=/usr/lib/pkgconfig
export OPENSSL_ROOT_DIR=/usr/local/ssl
export OPENSSL_LIBRARIES=/usr/local/ssl/lib/

PATH=/usr/local/ssl/bin:$PATH

export PATH=$PATH:/usr/local/cmake/bin

# apache maven
export M2_HOME=/home/openec/apache-maven-3.8.5
export PATH=$PATH:$M2_HOME/bin

# java
export JAVA_HOME=/usr/lib/jvm/jdk1.8.0_333
export JRE_HOME=${JAVA_HOME}/jre
export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib
export PATH=${JAVA_HOME}/bin:$PATH

# hadoop
export HADOOP_SRC_DIR=/home/openec/hadoop-3.0.0-src
export HADOOP_HOME=$HADOOP_SRC_DIR/hadoop-dist/target/hadoop-3.0.0
export PATH=$HADOOP_HOME/bin:$HADOOP_HOME/sbin:$PATH
export HADOOP_CLASSPATH=$JAVA_HOME/lib/tools.jar:$HADOOP_CLASSPATH
export CLASSPATH=`hadoop classpath --glob`
export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$JAVA_HOME/jre/lib/amd64/server/:/usr/local/lib:$LD_LIBRARY_PATH

source ~/.bashrc
```

### 安装cmake

```shell
tar -zxvf cmake-3.22.4.tar.gz
cd cmake-3.22.4/
./bootstrap
make
sudo make install

sudo ln -s /usr/local/bin/cmake /usr/bin/
```

### 安装redis

```shell
tar -zxvf redis-3.2.8.tar.gz
cd redis-3.2.8/
make
sudo make install

cd util/
sudo ./install_server.sh

sudo service redis_6379 stop
sudo sed -i 's/bind 127.0.0.1/bind 0.0.0.0/' /etc/redis/6379.conf
sudo service redis_6379 start
```

### 安装hiredis

```shell
tar -zxvf hiredis-1.0.2.tar.gz
cd hiredis-1.0.2/
make 
sudo make install
```

### 安装gf-complete

```shell
tar -zxvf gf-complete.tar.gz
cd gf-complete/
chmod +x ./autogen.sh
./autogen.sh
./configure
make
sudo make install
```

### 安装ISA-L

```shell
tar -zxvf isa-l-2.30.0.tar.gz
cd isa-l-2.30.0/
./autogen.sh
./configure
make
sudo make install
```

### 安装maven

```shell
tar -zxvf apache-maven-3.8.5-bin.tar.gz
mv apache-maven-3.8.5 ~/
```

### 安装java8

```shell
sudo mkdir /usr/lib/jvm
tar -zxvf jdk-8u333-linux-x64.tar.gz -C /usr/lib/jvm
```

### 安装hdfs-3

```shell
tar -zxvf hadoop-3.0.0-src.tar.gz
mv hadoop-3.0.0-src ~/
```

### 安装protobuf

```shell
tar -zxvf protobuf-2.5.0.tar.gz
cd protobuf-2.5.0/
./autogen.sh
./configure
make
sudo make install
```

### 安装openec

```shell
sudo cp /usr/include/x86_64-linux-gnu/zconf.h /usr/include/

tar -zxvf openec.tar.gz
cd openec/
cd hdfs3-integration/
./install.sh
```

