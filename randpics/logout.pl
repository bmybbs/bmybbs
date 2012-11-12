#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $remote_ip = $req -> remote_addr ();
print $req -> header ({-charset=>gb2312});
unless ($req -> cookie('id'))
{
    print "登陆超时!<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    die;
}#检查有没有cookie
my $id = $req -> cookie('id');
$id =~ /([a-z]*)/g;
my $username = $1;
$id =~ /([0-9]*)/g;
my $checknum = $1;
unless (-e "/tmp/$username.se")
{
    print "登陆超时<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
    die;
}#检查有没有session文件
unless (open (SE,"/tmp/$username.se"))
{
    print "打开文件失败<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
    die;
}
chomp (my $last_ip = <SE>);
chomp (my $randnum = <SE>);
chomp (my $acttime = <SE>);
my $nowtime = time;
unless (($last_ip == $remote_ip) && ($checknum == $randnum) && (($nowtime - $acttime) < 600))
{
    unlink ("/tmp/$username.se");
    print "登陆超时<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    close (SE);
    die;
}#检查有没有超时
close (SE);
unlink ("/tmp/$username.se");#如果没有超时则删除session文件
print "退出成功<br>";
