<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbstdoc?board=".$_GET['board']."&S=".$_GET['S'];
$result = file_get_html($proxy_url);

//output articles list
$article_list=$result->find('a[href^=bbstcon]');
$user_list=$result->find('a[href^=qry]');

//header parameters
$header_para=$result->find('a[target=f3]');

$page_list=$result->find('a[href^=bbstdoc]');

//output page bar
$page_flag=0;
$page_flag2=0;

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
	
	<div class="section small">文章列表</div>
	<?php
		$user_list_flag=count($user_list)-count($article_list);
		foreach($article_list as $ttoffset=>$tt) 
		{ 
			$tt->href="zarticle.php".substr($tt->href,7)."&S=".$_GET['S'];
			$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext);
			echo "<div class=\"small padtop\">".$tt;
			echo "-<a href='friend.php?u=".$user_list[$ttoffset+$user_list_flag]->innertext."'>".$user_list[$ttoffset+$user_list_flag]->innertext."</a>";
			echo "</div>";
		}
	?>
	<div class="divider_line"></div>
	
	<div class="small padtop">
		<?php
			foreach($page_list as $tt) 
			{ 
				  if($tt->innertext==iconv("UTF-8","GB2312//IGNORE","上一页")&&$page_flag==0)
				  {
					echo "<a href=\"zalist.php".substr($tt->href,7)."\">";
					echo iconv("GB2312","UTF-8//IGNORE",$tt->innertext)."</a>";
					$page_flag=1;
				   }
					if($tt->innertext==iconv("UTF-8","GB2312//IGNORE","下一页")&&$page_flag2==0)
				  {
					echo "|<a href=\"zalist.php".substr($tt->href,7)."\">";
					echo iconv("GB2312","UTF-8//IGNORE",$tt->innertext)."</a>";
					$page_flag2=1;
				  }
			}
		?>
	</div>
	<div class="small padtop">
		<a href='narticle.php<?php echo substr($header_para[1]->href,4)?>'>发表文章</a><br/>
	</div>
	<div class="small padtop">
		<a href='alist.php?B=<?php echo substr($header_para[1]->href,11)?>'>一般模式</a><br/>
	</div>
	<?php include('footer.php');?>
</body>
</html>
