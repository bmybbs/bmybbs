<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsmail";
if($_GET['S']>0||$_GET['t']=="t")
	$proxy_url=$proxy_url."?&S=".$_GET['S'];
	
$result = file_get_html($proxy_url);

//articles list
$article_list=$result->find('a[href^=bbsmailcon]');
$person_list=$result->find('a[href^=qry]');

//page bar
$article_pages=$result->find('a[href^=bbsmail?&S=],a[href^=mail?S]');
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
	<a class="small" href="index.php">首页</a></div>
	
	<div class="section small">信件列表</div>
	<?php
		foreach($article_list as $ttoffset=>$tt) 
		{ 
			$tt->href="mail.php".substr($tt->href,10);
			$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext)."-".$person_list[$ttoffset]->innertext;
			echo "<div class=\"small padtop\">".$tt."</div>";
			//echo "<a href=\"article.php".substr($tt->href,3)."&secstr=".$_GET['secstr']."&sec=".$_GET['sec']."\">";
		}
	?>
	<div class="divider_line"></div>
	<div class="small padtop">
		<?php
			foreach($article_pages as $aoffset=>$article_page)
			{
				if(strstr($article_page->href,"bbsmail?&S"))
					echo "<a href=\"maillist.php?".substr($article_page->href,9)."\">";
				else
					echo "<a href=\"maillist.php?t=t&".substr($article_page->href,5)."\">";
					
				echo iconv("GB2312","UTF-8//IGNORE",$article_page->innertext)."</a>";	
				if($aoffset<count($article_pages)-1)
					echo "|";
			}
		?>
	</div>
	<div class="small padtop">
		<a href='nmail.php'>发送信件</a>
	</div>
	<?php include('footer.php');?>
</body>
</html>
