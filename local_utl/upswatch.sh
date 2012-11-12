#!/bin/sh
#利用监视网关是否通畅来判断是否停电 如果停电 就自动停止bbs服务 等待来电
#以下是网关地址和服务信息 你可以修改这个
GATE=162.105.31.1
SERVICE="syslog crond inet named sendmail httpd proftpd mysql atd"
BBSSERV="bbsd bbs yftpd bbsinnd bbspop3d innbbsd"
BBSUSER=bbs
#internal variable
n=60
STOP=0

function stopbbs() {
	#停止所有bbs服务进程
	for i in $BBSSERV;do
		killall $i #为了防止意外 多次kill
		sleep 2
		killall -9 $i
	done
	#停止所有系统服务
	for i in $SERVICE;do
		/etc/rc.d/init.d/$i stop #为了防止意外 多次停止
		sleep 2
		/etc/rc.d/init.d/$i stop
		sleep 2
		/etc/rc.d/init.d/$i stop
	done
	#停止其它以bbs用户身份运行的程序 比如挖地雷 查单词等
	for i in `ps -u $BBSUSER|awk '{print $1}'`;do
		kill $i
		sleep 2
		kill -9 $i
	done
	#umount所有分区 同时把/ mount为只读 等待停电
	umount -a
	sync;sleep 2;sync;sleep 2;sync
	mount / -o remount,ro
}

function startbbs() {
	#mount文件系统
	mount / -o remount,rw
	sleep 1
	mount -a
	#启动系统服务 注意因为前面stopbbs时候shm并没有清除 所以httpd可以先于bbsd起
	for i in $SERVICE;do
		/etc/rc.d/init.d/$i start
		sleep 2
	done
	#这里是bbs的服务程序 因为各自需要的用户不同 所以只好用这种直接写到脚本的方式 谁能改改?
	/home/bbs/bin/bbsd
	/home/bbs/bin/bbspop3d
	su bbs -c "/home/bbs/ftphome/yftpd"
	su bbs -c "/home/bbs/bin/bbslogd"
	su bbs -c "/home/bbs/bin/bbsinnd"
	su bbs -c "/home/bbs/innd/innbbsd"
}
#这里是主框架
#==============main function start here=======================================
while true;do
	if ping $GATE -c $n -w $n -n | grep "100% packet loss" &> /dev/null ; then
		if [ $STOP -ne 1 ];then
			STOP=1
			stopbbs
		fi
	else
		if [ $STOP -eq 1 ];then
			STOP=0
			startbbs
		fi
	fi
	sleep 10
done
#==============shell script end===============================================
