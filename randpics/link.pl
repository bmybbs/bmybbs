#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $host = "http://202.117.1.8";
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $remote_ip = $req -> remote_addr ();
print $req -> header ({-charset=>gb2312});
unless ($req -> cookie('id'))
{
    print "登陆超时，请重新登陆!<br>";
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
    print "登陆超时，请重新登陆<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
    die;
}#检查有没有session文件
unless (open (SE,"+</tmp/$username.se"))
{
    print "打开文件失败<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
    die;
}
chomp (my $last_ip = <SE>);
chomp (my $randnum = <SE>);
chomp (my $acttime = <SE>);
my $nowtime = time;
unless (($last_ip eq $remote_ip) && ($checknum == $randnum) && (($nowtime - $acttime) < 600))
{
    print "登陆超时，请重新登陆<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    close (SE);
    die;
}#检查有没有超时
seek (SE,length ($randnum) + length ($last_ip) + 2,0) || die;
print SE time;#更新最后活动时间
close (SE);
if ($req -> param ())
{
    my $newboard = $req -> param ("BOARD");
    open (LINK,">$bbshome/loglink");
    print LINK "BMYKWDVKPBGPAAXVDSNNLFFOMPPOKRCXYYMQ_B/home?B=",$newboard;
    close (LINK);
    print "修改成功<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
}
else
{
    print "没有操作<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
}
