#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $username = $req -> param ("UN");
my $password = $req -> param ("PW");
my $passfile = "/home/bbs/.PASSWDS";
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "http://202.117.1.8/picmgr.htm";
my $lock_sh = 1;
my $lock_un = 8;
my $randnum = int (rand () * 1000000000000);

# 一些常量，在升级 .PASSWDS 结构后需要对应更新
my $IDLEN = 14;
my $PWLEN = 14;
my $SIZE_OF_USEREC = 528;
my $OFFSET_OF_PW = 74;

while ($randnum <= 100000000000)
{
    $randnum = int (rand () * 1000000000000);
}
$username = lc ($username);
my $cookie = $username.$randnum;
print "Set-Cookie:id=$cookie\n";
print $req -> header ({-charset=>gb2312});
if (!$username || $username =~ m/[^a-z]/ || $password =~ m/\s/)
{
    print "错误!ID只能包含字母，密码不能包含空格!";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    die;
}#用正则表达式对输入的ID和密码过滤
open (AD,"$bbshome/artdesign") || die;
chomp (my $allow = <AD>);
while (lc ($allow) ne $username)
{
    chomp ($allow = <AD>);
    unless ($allow)
    {
	print "没有登陆权限,请联系技术站长";
	close (AD);
	die;
    }
}#判断有没有美工组权限
close (AD);
open (PW,$passfile) || die "错误！打开文件$passfile失败!";
until (flock (PW,$lock_sh))
{
    sleep 1;
}
read (PW,$id,$IDLEN);
until (($lastchar = chop ($id)) ne "\0")
{
    ;
}
$id .= $lastchar;
while (lc ($id) ne $username)
{
    seek (PW,$SIZE_OF_USEREC - $IDLEN,1);
    read (PW,$id,$IDLEN);
    unless ($id)
    {
	print "无此ID,请重新登陆";
	print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
	close (PW);
	die;
    }
    until (($lastchar = chop ($id)) ne "\0")
    {
	;
    }
    $id .= $lastchar;
}#先找ID
seek (PW,$OFFSET_OF_PW - $IDLEN,1);
read (PW,$passwd,$PWLEN);
until (($lastchar = chop ($passwd)) ne "\0")
{
    ;
}
$passwd .= $lastchar;
until (flock (PW,$lock_un))
{
    sleep 1;
}#再找密码
close (PW);
my $salt = substr ($passwd,0,2);
my $chkpw = crypt ($password,$salt);
if ($chkpw eq $passwd)#判断密码是否正确
{
    open (SE,">/tmp/$username.se") || die "错误！无法创建session文件!";
    my $acttime = time;
    my $remote_ip = $req -> remote_addr ();
    print SE $remote_ip,"\n";
    print SE $randnum,"\n";
    print SE $acttime,"\n";
    close (SE);
    print "登陆成功";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
}
else
{
    print "登陆失败，请检查ID和密码，或者联系技术站长。<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
}
