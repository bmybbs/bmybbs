#!/usr/bin/perl
use CGI;
print "Content-type: text/html\n\n";
my $req = new CGI;
my $username = $req->param("UN");
my $filename = "/home/bbs/$username.pw";
if (-e $filename){
	unless (open (PW,"/home/bbs/$username.pw")){
		print "错误！打开文件$username.pw失败";
		die;
		}
	chomp (my $oldpw = <PW>);
	my $chkpasswd = $req->param("OP");
	if ($chkpasswd eq $oldpw){
		close (PW);
		my $newpw = $req->param("NP1");
		my $newpw2 = $req->param("NP2");
		unless ($newpw eq $newpw2){
			print "错误！密码不匹配";
			die;
			}
		unless (open (PW,">/home/bbs/$username.pw")){
			print "错误！写入文件$username.pw失败";
			die;
			}
		print PW "$newpw\n";
		close (PW);
		print "密码修改成功";
		}
	else{
		print "旧密码错误！";
		}
	}
else{
	print "错误！用户$username不存在";
	die;
	}
