#!/usr/bin/perl -w
use CGI;
my $req = new CGI;
my $upfilecount = 1;
my $maxupload = 5;
my $ext = ".jpg";
my $bbshome = "/home/bbs";
my $htmpath = "/home/apache/htdocs/bbs/bmyMainPic";
my $loginadd = "/picmgr.htm";
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
print $req -> start_html (),
    $req -> h2 ("Welcome to XJTU bbs BMY!"),
    $req -> a ({-href=>"showpics.pl"},"返回"),
    "|",
    $req -> a ({-href=>"logout.pl"},"退出"),
    $req -> hr (),
    $req -> start_multipart_form ("POST","","gb2312");
for (my $i = 1;$i <= $maxupload;$i ++)
{
    print $req -> filefield ({-name=>"FILE$i"}),
    $req -> br (),
}
print $req -> submit ({-label=>'upload'}),
    $req -> end_form,
    $req -> hr ();#生成上传文件的表单
if ($req -> param ())
{
    while ($upfilecount <= $maxupload)
    {
	my $file = $req -> param ("FILE$upfilecount");
	if ($file)
	{
	    $filename = $file;
	    $filename =~ s/^.*(\\|\/)//; #用正则表达式去除路径名，得到文件名
	    if ((-e "$htmpath/uploaded/$filename") || (-e "$htmpath/using/$filename") || (-e "$htmpath/used/$filename"))
	    {
		$message .= "文件$filename已经存在，请重命名后再重新上传<br>\n";
		$upfilecount ++;
		next;
	    }
	    my $extname = lc (substr ($filename,length ($filename) - 4,4));
	    unless ($extname ne $ext)
	    {
		open (OUTFILE, ">$htmpath/uploaded/$filename");
		while (read ($file,my $buffer,1024))
		{
		    print OUTFILE $buffer;
		}
		close (OUTFILE);
		$message .= "$filename已成功上传<br>\n";
	    }
	    else
	    {
		$message .= "$filename上传失败,只接受jpg文件<br>\n";
	    }
	}
	$upfilecount ++;
    }
}
unless ($message)
{
    $message .= "请选择要上传的文件";
}
print $message; #最后输出上传信息
print $req -> end_html;
