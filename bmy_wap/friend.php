<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsqry?userid=".$_GET['u'];
$result = file_get_html($proxy_url);
$friend_info=$result->find('pre',0);
$shuoming=$result->find('td',0);
$article_pics=$shuoming->find("img,IMG");

$cm=new CreatMiniature();
foreach($article_pics as $tt)
{	
		if($tt->src=="")
			$article_pic=$tt->SRC;
		else
			$article_pic=$tt->src;
		srand((double)microtime()*1000000); 
		$randval = rand(0,999); 
		
		$cm->SetVar($article_pic,"file");
		$new_pic_name=time().$randval.".jpg";
		@$cm->Prorate("tempimg/".$new_pic_name,150,200);
		if(strstr($article_pic,"&amp;")&&strstr($article_pic,"attachname="))
			$article_pic_t="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/".str_replace("&amp;","&",$article_pic);
		else
			$article_pic_t=$article_pic;
		$shuoming->innertext=str_replace($article_pic_t,"tempimg/".$new_pic_name,$shuoming->innertext);
		//article_pic="tempimg/".$new_pic_name;
}

$shuoming=str_replace("<br>","<br/>",$shuoming->innertext);

$friend_info_temp=$friend_info->innertext;
$friend_info=myfind("$$$".$friend_info_temp,"$$$","</pre>",0);
$friend_info_out=str_replace("<font class=c33>","",$friend_info[0]);
$friend_info_out=str_replace("<font class=c37>","",$friend_info_out);
$friend_info_out=str_replace("<font class=b40>","",$friend_info_out);
$friend_info_out=str_replace("class=h32","class=\"green\"",$friend_info_out);
$friend_info_out=str_replace("class=c32","class=\"green\"",$friend_info_out);
$friend_info_out=str_replace("class=h36","class=\"darkred\"",$friend_info_out);
$friend_info_out=str_replace("class=c35","class=\"red\"",$friend_info_out);
$friend_info_out=str_replace("<br>","",$friend_info_out);

$friend_info_out=str_replace(") ",") <br/>",$friend_info_out);
$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","，发表文章"),iconv("UTF-8","GB2312//IGNORE","<br/>发表文章"),$friend_info_out);
$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","上次在"),iconv("UTF-8","GB2312//IGNORE","<br/>上次在"),$friend_info_out);
$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","] 从 ["),iconv("UTF-8","GB2312//IGNORE","] <br/>从 ["),$friend_info_out);
$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","信箱："),iconv("UTF-8","GB2312//IGNORE","<br/>信箱："),$friend_info_out);
$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","目前在站上, 状态如下:"),iconv("UTF-8","GB2312//IGNORE","<br/>目前在站上, 状态如下:<br/>"),$friend_info_out);

$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","目前不在站上, 上次离站时间"),iconv("UTF-8","GB2312//IGNORE","<br/>目前不在站上, 上次离站时间<br/>"),$friend_info_out);

$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","担任版务"),iconv("UTF-8","GB2312//IGNORE","<br/>担任版务"),$friend_info_out);

$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","★<font class=\"darkred\">"),iconv("UTF-8","GB2312//IGNORE","★"),$friend_info_out);

$friend_info_out=str_replace("href=bbshome","href=alist.php",$friend_info_out);

$friend_info_out=str_replace("target=f3","",$friend_info_out);

$friend_info_out=str_replace(iconv("UTF-8","GB2312//IGNORE","202.117.1.86"),iconv("UTF-8","GB2312//IGNORE","wap登陆"),$friend_info_out);
?> 
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.0//EN" "http://www.wapforum.org/DTD/xhtml-mobile10.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title><?php echo $global_webtitle?></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>
<link rel="stylesheet" href="css/main.css" type="text/css"/>
<body>
	<?php echo $global_webtop?>
	<br/><br/>
	<?php include('logincheck.php');?>
	<div class="padbottom">
		<a class="small" href="index.php">首页</a>
	</div>
	
	<div class="section small">查询网友</div>
	<div class="small padtop">
		<?php
			echo iconv("GB2312","UTF-8//IGNORE",$friend_info_out);
			if($friend_info_out=="")
				echo "无此用户";
			echo "<br><br>";
			echo iconv("GB2312","UTF-8//IGNORE",$shuoming);	
		?>
	</div>
	<?php
		if($friend_info_out!="")
		{
	?>
	<div class="divider_line"></div>
	<div class="small padtop">
		<a href='nmail.php?touser=<?php echo $_GET['u']?>'>书灯絮语</a><br/>
	</div>
	<div class="small padtop">
		<a href='<?php echo $_SERVER['HTTP_REFERER']?>'>返回上页</a><br/>
	</div>
	<?php
		}
		include('footer.php');
	?>
</body>
</html>
