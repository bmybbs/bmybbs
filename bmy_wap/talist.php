<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/tfind?B=".$_GET['B']."&th=".$_GET['th']."&T=".$_GET['T'];
$result = file_get_html($proxy_url);

//output articles list
$article_list=$result->find('a[href^=qry],a[href^=con]');

//parameters
$back_board=$result->find('a[href^=bbsdoc]',0);
$ztzk_board=$result->find('a[href^=bbstcon]',0);

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
	<div class="section small">
	   <?PHP
			 $ztzk_board->href="zarticle.php".substr($ztzk_board->href,7);
			 $ztzk_board->innertext=iconv("GB2312","UTF-8//IGNORE",$ztzk_board->innertext);
			 
			 echo "<a href='";
			 echo $ztzk_board->href;
			 echo "'>";
			 echo $ztzk_board->innertext;
			 echo "</a>";
		?>
	</div>
	<?php
		foreach($article_list as $ttoffset=>$tt) 
		{ 
	        if($ttoffset%2==0)
			{
			echo "<div class=\"small padtop\">";
			
			$article_list[$ttoffset+1]->href="article.php".substr($article_list[$ttoffset+1]->href,3);
			$article_list[$ttoffset+1]->innertext=iconv("GB2312","UTF-8//IGNORE",$article_list[$ttoffset+1]->innertext);
			echo $article_list[$ttoffset+1];
			
			//$tt->href="zarticle.php".substr($tt->href,7);
			$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext);
			echo "<a href='friend.php?u=".$tt->innertext."'>".$tt->innertext."</a>";
		
			echo "</div>";
			}
		}
	?>
	<div class="divider_line"></div>
	<div class="small padtop">
		<?PHP
			 $back_board->href="alist.php?B".substr($back_board->href,12);
			 $back_board->innertext=iconv("GB2312","UTF-8//IGNORE",$back_board->innertext);
		 
			 echo "<a href='";
			 echo $back_board->href;
			 echo "'>";
			 echo $back_board->innertext;
			 echo "</a>";
		?>
	</div>
	<div class="small padtop">
		<?PHP
			 echo "<a href='";
			 echo $ztzk_board->href;
			 echo "'>";
			 echo $ztzk_board->innertext;
			 echo "</a>";
		?>
	</div>
	<?php include('footer.php');?>
</body>
</html>
