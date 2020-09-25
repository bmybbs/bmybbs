#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs/bmyMainPic";
my $cgibin = "http://202.117.1.8/cgi-bin/bbs";
my $loginadd = "/picmgr.htm";
my $remote_ip = $req -> remote_addr ();
print $req -> header ({-charset=>'gb2312'});
if ($req -> cookie('id'))#检查有没有cookie
{
    my $id = $req -> cookie('id');
    $id =~ /([a-z]*)/g;
    my $username = $1;
    $id =~ /([0-9]*)/g;
    my $checknum = $1;
    unless (-e "/tmp/$username.se")#检查有没有session文件
    {
	print "登陆超时，请重新登陆<br>";
	print "<meta http-equiv=\"refresh\" content=\"2; URL=$loginadd\">";
	die;
    }
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
    if (($last_ip eq $remote_ip) && ($checknum == $randnum) && (($nowtime - $acttime) < 600))
    {
	seek (SE,length ($randnum) + length ($last_ip) + 2,0) || die;
	print SE time;#如果没有超时则更新最后活动时间
	close (SE);
	if ($req -> param ())
	{
	    open (PIC,$bbshome."/logpics") || die "错误!打开文件列表失败!";
	    chomp (my $picnum = <PIC>);
	    $picnum =~ s/\D//g;
	    my $del = 0;#记录删除文件数组的下标
	    my $new = 0;#记录保留文件数组的下标
	    for (my $i = 0;$i < $picnum;$i ++)
	    {
		chomp ($pics[$i] = <PIC>);
		if ($req -> param ($i))
		{
		    $delpics[$del ++] = $pics[$i];
		}
		else
		{
		    $newpics[$new ++] = $pics[$i];
		}
	    }#把删除文件和保留文件分别存在两个数组里
	    close (PIC);
	    open (PIC,">$bbshome/logpics") || die $!;
	    print PIC "total:".$new,"\n";
	    for ($i = 0;$i < $new;$i ++)
	    {
		print PIC $newpics[$i],"\n";
	    }#把保留文件列表写入文件保存
	    close (PIC);
	    for ($i = 0;$i < $del;$i ++)
	    {
		rename ($htmpath."/using/".$delpics[$i],$htmpath."/used/".$delpics[$i]) || die $!;
	    }#把删除文件移到used文件夹
	    print "删除成功<br>";
	    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
	}
	else
	{
	    print "没有操作……<br>";
	    print "<meta http-equiv=\"refresh\" content=\"2; url=$cgibin/showpics.pl\">";
	}
    }#检查登陆是否已经超时
    else
    {
	close (SE);
	print "登陆超时，请重新登陆<br>";
	print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
    }
}
else
{
    print "登陆超时，请重新登陆<br>";
    print "<meta http-equiv=\"refresh\" content=\"2; url=$loginadd\">";
}
