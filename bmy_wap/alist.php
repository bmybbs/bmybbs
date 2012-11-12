<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/home?B=".$_GET['B']."&S=".$_GET['S'];
$result = file_get_html($proxy_url);

//错误
if(strstr($result->innertext,iconv('UTF-8', 'gb2312',"没有这个讨论区啊")))
    header("location:index.php");

//articles list
$article_list=$result->find('.tdborder a');
$user_list=$result->find('td[class=tduser] a');
$unread=$result->find('table[cellpadding=2]',0);
$unread=$unread->find('tr');
//header parameters
$header_para=$result->find('a[target=f3]');

//page bar
$article_pages=$result->find('td[align=right] a');
$page_flag=0;
$page_flag2=0;

//online person
$online_person=myfind($result->innertext,iconv('UTF-8', 'gb2312',"在线&lt;"),"&gt;",0);
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
		<a class="small" href="index.php">首页</a><span class="small">>></span><a class="small" href="<?php echo "board.php".substr($header_para[0]->href,3)?>"><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[0]->innertext) ?></a><span class="small">>><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[1]->innertext)?></span>
	</div>
	
	<div class="section small">文章列表(<?php echo "在线".$online_person[0]?>)</div>
	<?php
		foreach($article_list as $ttoffset=>$tt) 
		{ 	
			$unread_t="";
			if($unread[$ttoffset+1]->class!="red")
			{
				$unread_temp=$unread[$ttoffset+1]->find('td');
				$unread_t=str_replace("N","*",$unread_temp[1]->innertext);
				$tt->href="article.php".substr($tt->href,3)."&S=".$_GET['S'];
				$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext);
				echo "<div class=\"small padtop\">".$unread_t.$tt."-<a href='friend.php?u=".$user_list[$ttoffset]->innertext."'>".$user_list[$ttoffset]->innertext."</a></div>";
			}
			else
			{
				if($_GET['S']==0)
				{
					$tt->href="article.php".substr($tt->href,3)."&S=".$_GET['S'];
					$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext);
					echo "<div class=\"small padtop\">".$tt."-<a href='friend.php?u=".$user_list[$ttoffset]->innertext."'>".$user_list[$ttoffset]->innertext."</a></div>";
				}
			}
		}
	?>
	<div class="divider_line"></div>
	
	<div class="small padtop">
		<?php
			foreach($article_pages as $article_page)
			{
				if($article_page->innertext==iconv("UTF-8","GB2312//IGNORE","第一页"))
					$page_flag++;
				if($page_flag>1)
				{
					echo "<a href=\"alist.php".substr($article_page->href,3)."\">";
					echo iconv("GB2312","UTF-8//IGNORE",$article_page->innertext)."</a>";
					$page_flag2++;
					if($page_flag2<4)
						echo "|";			
				}
			}
		?>
	</div>
	<div class="small padtop">
		<a href='narticle.php<?php echo substr($header_para[1]->href,4)?>'>发表文章</a><br/>
	</div>
	<div class="small padtop">
		<a href='zalist.php<?php echo substr($header_para[1]->href,4)?>'>主题模式</a><br/>
	</div>
	<?php include('footer.php');?>
</body>
</html>
