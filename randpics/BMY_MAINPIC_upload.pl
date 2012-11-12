#!/usr/bin/perl
use CGI;
$basedir = "/home/apache/htdocs/bmy/"; #上传的文件存放地址
print "Content-type: text/html\n\n";
my $req = new CGI;
my $username = $req->param("USERNAME");
my $chkpasswd = $req->param("PASSWD");
unless (open (PW,"/home/apache/htdocs/bmy/BMY_MAINPIC/$username.pw")){
	print "错误！用户$username不存在";
	die;
	}
chomp ($passwd = <PW>);
close (PW);
if ($chkpasswd eq $passwd){
	my $file = $req->param("FILE");
	if ($file ne "") {
		$fileName = $file;
		$fileName =~ s/^.*(\\|\/)//; #用正则表达式去除无用的路径名，得到文件名
		my $newmain = $fileName;
		$filenotgood = "no";
		if ($fileName ne "cai.jpg"){
			$filenotgood = "yes";
			}
		}
	chomp (my $used = `date +%F-%R`);
	system "mv /home/apache/htdocs/bmy/cai.jpg /home/apache/htdocs/bmy/BMY_MAINPIC/used/$used.jpg";
	if ($filenotgood ne "yes") { #这段开始上传
		open (OUTFILE, ">$basedir/$fileName");
		binmode(OUTFILE); #务必全用二进制方式，这样就可以放心上传二进制文件了。而且文本文件也不会受干扰
		while (my $bytesread = read($file, my $buffer, 1024)) {
			print OUTFILE $buffer;
            		}
            	close (OUTFILE);
            	$message.="cai.jpg 已成功上传,原文件保存为$used.jpg<br>\n";
        	}
        else{
            $message.="上传失败，请把文件名改成cai.jpg重新上传<br>\n";
	}
}
else{
$message.="密码不正确，请联系技术站长 <br>\n";
}
print $message; #最后输出上传信息
