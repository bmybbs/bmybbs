# BMYBBS ![Backend Build Status](https://github.com/bmybbs/bmybbs/workflows/BMYBBS%20Backend/badge.svg) ![Frontend Build Status](https://github.com/bmybbs/bmybbs/workflows/BMYBBS%20Frontend/badge.svg) [![Coverity Scan Build Status](https://scan.coverity.com/projects/4511/badge.svg)](https://scan.coverity.com/projects/4511)

**BMYBBS** 是 [YTHT](https://zh.wikipedia.org/wiki/%E4%B8%80%E5%A1%8C%E7%B3%8A%E6%B6%82BBS) 代码的一个分支，被使用在[西安交通大学兵马俑 BBS](http://bbs.xjtu.edu.cn)。 当前开发和运行环境使用的是 Ubuntu 18.04 / gcc 7.5.0，测试环境使用的是 Ubuntu 20.04 / gcc 10.2.0。

BMYBBS 已经和最初的 YTHT 系统不兼容。由于能力和精力有限，本项目未提供升级或者转换程序，同时代码中或许移除掉了一些可移植性相关的宏或者代码，因此本项目亦不能保证在其他 Linux 发行版或者 POSIX 系统上运行。

如果您仅需要反馈问题，可以前往 [issues](https://github.com/bmybbs/bmybbs/issues) 查找是否已被反馈，或者提交新的 issue 。如果对于 BMYBBS 的开发、运行感兴趣，以下指南或许可以给您帮助。

## 环境准备

### 工具和库

在编译过程中需要使用到的一些工具和库

```bash
sudo apt install build-essential libtool cmake gdb chrpath \
	apache2 libapache2-mod-perl2 mysql-server redis-server \
	libmysqlclient-dev libpcre3-dev libjson-c-dev libhiredis-dev \
	libxml2-dev libgmp-dev libcurl4-openssl-dev
```

以下工具亦可能带来帮助：

```bash
sudo apt install language-pack-zh-hans git zsh tmux
```

本项目还依赖于 libghthash、[onion](https://github.com/davidmoreno/onion) 以及 [check](https://github.com/libcheck/check) ，相关编译安装可以在 [travis-ci 配置文件](.travis.yml) 找到。

### 用户环境

BMYBBS 运行使用 bbs 用户身份，其 UID/GID 均为 999。如果系统中已有用户和组使用了这些 ID，例如在 Ubuntu 20.04 中被 `systemd-coredump` 占用，可以参照如下命令：

```bash
sudo groupmod -g 1999 systemd-coredump
sudo usermod  -u 1999 systemd-coredump
```

使用如下命令创建 bbs 用户和用户组：

```bash
sudo groupadd -g 999 bbs
sudo adduser --uid 999 --gid 999 bbs
```

### 初始化 bbs 相关目录和配置文件
以下是典型的使用，其中某些值可能已经作为常量写入了代码，建议除了 `LOCALIP` 外保持一致。

```bash
BBS_HOME=/home/bbs \
HTMPATH=/home/apache/htdocs/bbs \
CGIPATH=/home/apache/cgi-bin/bbs \
LOCALIP=202.117.1.8 \
sudo -E ./PrepDirs.sh
```

## 编译

```bash
# 假设您已经在 bmybbs 代码根目录
mkdir build
cd build
cmake ..
make
```

这里的编译应该是成功的。单元测试中一些验证码相关的用例会因为您缺少数据而不通过，因此您可以忽略执行单元测试。

> **已知问题**: 当前使用 cmake 构建的 [sshbbsd](smth_sshbbsd/) 会在建立连接后断开并终止子进程，因此需要使用原先的 Makefile 方式。建议参考 [编译sshbbsd的过程.txt](doc/System_Maintenance/编译sshbbsd的过程.txt) 。

## 安装

参考 [install_binaries_and_libraries.sh](install_binaries_and_libraries.sh)，典型使用方法：

```bash
# 整体安装，此处假设您位于上一步编译的目录下（即 build 目录）
../install_binaries_and_libraries.sh .

# 单独更新，例如 libytht.so 和 bbsd，此处假设您位于上一步编译的目录下（即 build 目录）
../install_binaries_and_libraries.sh libytht/libytht.so /home/bbs/lib/libytht.so
../install_binaries_and_libraries.sh src/bbsd/bbsd /home/bbs/bin/bbsd
```

## 额外配置

### apache2

1. 在 `/etc/apache2/envvars` 中设置 `APACHE_RUN_USER` 和 `APACHE_RUN_GROUP` 为 `bbs`。
2. 使用 `a2enmod(8)` 启用 `rewrite` 和 `cgid` 两个模块。

重启 apache2 以生效。

### 系统调试

BBS 存在很多缺陷，难免会意外终止程序，这时借助 gdb 和转储文件可以帮助很多。参考以下配置：

```bash
ulimit -c unlimited
sysctl -w kernel.core_pattern='/core/%e.%t.%u.%g.%s.%p'
```

## 其他

如果您在编译安装中遇到未提及的情况，可能在如下文档中能找到帮助信息：

* [BMYBBS权威安装文档.txt](doc/System_Maintenance/BMYBBS权威安装文档.txt)
* [nju09如何使用debian自带apache2.2.txt](doc/System_Maintenance/[文档]nju09如何使用debian自带apache2.2.txt)

