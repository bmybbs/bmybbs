#!/bin/sh
# 安装可执行文件和库，这里就不做输入校验了，这里安装的含义除了将文件放置在对应位置外，
# 还包括移除调试信息、设置权限的含义。因此安装之后一定要保留原二进制文件，用于处理调
# 试信息。
# 两种使用方式
# - ./install_binaries_and_libraries.sh /path/to/build/dir
#   当只有一个参数时，即编译构建的目录，安装全部的库和可执行程序
# - ./install_binaries_and_libraries.sh /path/to/src /path/to/dest
#   当超过一个参数时，去第一个参数作为源地址，第二个参数为目标地址，用于安装特定文件


bmy_install() {
	SRC=$1
	TARGET=$2
	echo "Installing $TARGET"
	cp $SRC $TARGET.new
	chrpath -r /home/bbs/lib $TARGET.new 1>/dev/null
	install -s -m 550 $TARGET.new $TARGET
	rm $TARGET.new
}

install_library() {
	bmy_install $1 /home/bbs/lib/$2
}

install_binary() {
	bmy_install $1 /home/bbs/bin/$2
}

install_www() {
	bmy_install $1 /home/apache/cgi-bin/bbs/$2
}
if [ "$#" -ne 1 ]; then
	# 单独安装一个文件，使用绝对路径
	bmy_install $1 $2
else
	BUILD_DIR=$1

	install_library $BUILD_DIR/libytht/libytht.so       libytht.so
	install_library $BUILD_DIR/libbmy/libbmy.so         libbmy.so
	install_library $BUILD_DIR/libythtbbs/libythtbbs.so libythtbbs.so

	install_binary $BUILD_DIR/src/bbs/bbs               bbs
	install_binary $BUILD_DIR/src/bbsd/bbsd             bbsd
	install_binary $BUILD_DIR/src/bbs.chatd/bbs.chatd   bbs.chatd
	install_binary $BUILD_DIR/src/bbsnet/bbsnet         bbsnet
	install_binary $BUILD_DIR/src/telnet/telnet         telnet
	install_binary $BUILD_DIR/src/thread/thread         thread

	install_binary $BUILD_DIR/atthttpd/atthttpd         atthttpd
	install_www    $BUILD_DIR/nju09/www/www             www

	install_binary $BUILD_DIR/local_utl/averun/averun                         averun
	install_binary $BUILD_DIR/local_utl/auto_rm_junk/auto_rm_junk             auto_rm_junk
	install_binary $BUILD_DIR/local_utl/autoclear/autoclear                   autoclear
	install_binary $BUILD_DIR/local_utl/autoundeny/autoundeny                 autoundeny
	install_binary $BUILD_DIR/local_utl/bbslogd/bbslogd                       bbslogd
	install_binary $BUILD_DIR/local_utl/bbspop3d/bbspop3d                     bbspop3d
	install_binary $BUILD_DIR/local_utl/bbsstatlog/bbsstatlog                 bbsstatlog
	install_binary $BUILD_DIR/local_utl/bbsstatproclog/bbsstatproclog         bbsstatproclog
	install_binary $BUILD_DIR/local_utl/bbstop/bbstop                         bbstop
	install_binary $BUILD_DIR/local_utl/bm/bm                                 bm
	install_binary $BUILD_DIR/local_utl/changeboardname/changeboardname       changeboardname
	install_binary $BUILD_DIR/local_utl/check_ulevel/check_ulevel             check_ulevel
	install_binary $BUILD_DIR/local_utl/clear_attach/clear_attach             clear_attach
	install_binary $BUILD_DIR/local_utl/clear_junk/clear_junk                 clear_junk
	install_binary $BUILD_DIR/local_utl/combine_arc/combine_arc               combine_arc
	install_binary $BUILD_DIR/local_utl/cpersonal/cpersonal                   cpersonal
	install_binary $BUILD_DIR/local_utl/find_lost_mail/find_lost_mail         find_lost_mail
	install_binary $BUILD_DIR/local_utl/find_rm_lost/find_rm_lost             find_rm_lost
	install_binary $BUILD_DIR/local_utl/finddf/finddf                         finddf
	install_binary $BUILD_DIR/local_utl/fixdir/fixdir                         fixdir
	install_binary $BUILD_DIR/local_utl/id_boards/id_boards                   id_boards
	install_binary $BUILD_DIR/local_utl/makeindex3/makeindex3                 makeindex3
	install_binary $BUILD_DIR/local_utl/mergeb/mergeb                         mergeb
	install_binary $BUILD_DIR/local_utl/nbstat/nbstat                         nbstat
	install_binary $BUILD_DIR/local_utl/newboards/newboards                   newboards
	install_binary $BUILD_DIR/local_utl/newtop10/newtop10                     newtop10
	install_binary $BUILD_DIR/local_utl/postfile/postfile                     postfile
	#install_binary $BUILD_DIR/local_utl/printSecLastMark/printSecLastMark     printSecLastMark
	install_binary $BUILD_DIR/local_utl/printSecLastUpdate/printSecLastUpdate printSecLastUpdate
	install_binary $BUILD_DIR/local_utl/ptyexec/ptyexec                       ptyexec
	install_binary $BUILD_DIR/local_utl/repsync/repsync                       repsync
	install_binary $BUILD_DIR/local_utl/save_brc/save_brc                     save_brc
	install_binary $BUILD_DIR/local_utl/searchDIR/searchDIR                   searchDIR
	install_binary $BUILD_DIR/local_utl/searchLastMark/searchLastMark         searchLastMark
	install_binary $BUILD_DIR/local_utl/selpersonal/selpersonal               selpersonal
	install_binary $BUILD_DIR/local_utl/setdefaultkey/setdefaultkey           setdefaultkey
	install_binary $BUILD_DIR/local_utl/sortdir/sortdir                       sortdir
	install_binary $BUILD_DIR/local_utl/transuu2bin/transuu2bin               transuu2bin
	install_binary $BUILD_DIR/local_utl/watchman/watchman                     watchman
fi
