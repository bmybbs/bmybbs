<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsmailcon?file=".$_GET['file']."&num=".$_GET['num'];
$result = file_get_html($proxy_url);

//gete all the data needed
$article_all=$result->find('.bordertheme',0)->innertext;
$article_all=str_replace("<br>","<br/>",$article_all);
$article_all=substr($article_all,0,strlen($article_all)-18);
$article_title=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","标 &nbsp;题: "),"<br/>",0);
$article_content=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","来 &nbsp;源: "),"<br/>--",0);
$article_date=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","发信站: 兵马俑BBS ("),")",0);
$article_author=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","寄信人: "),"<br/>",0);

$userfrom=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","寄信人: ")," (",0);

//change the pics
$article_pics=myfind($article_content[0],"<img src='","'>",0);
//echo "<img src='".$article_pics[0]."'>";

//$t = new ThumbHandler();
$cm=new CreatMiniature();
foreach($article_pics as $article_pic)
{	
 	if($article_pic!="/images/spacer.gif")
	{	
	   //$new_pic_name=time().".jpg";
	   //$t->setSrcImg($article_pic->src);
       //$t->setMaskWord("wap");
	   //$t->setDstImg("tempimg/".$new_pic_name);
       //$t->createImg(200,200);
	   //$article_pic->src="tempimg/".$new_pic_name;
	 	
		srand((double)microtime()*1000000); 
		$randval = rand(0,999); 
		
		$cm->SetVar($article_pic,"file");
		$new_pic_name=time().$randval.".jpg";
		$cm->Prorate("tempimg/".$new_pic_name,150,200);
		$article_content[0]=str_replace($article_pic,"tempimg/".$new_pic_name,$article_content[0]);
		//article_pic="tempimg/".$new_pic_name;
	}
}

//output footer
$last_page=$result->find('a[accesskey=f]',0);
$next_page=$result->find('a[accesskey=n]',0);
//$mail_back=$result->find('a[accesskey=m]',0);

//echo "<br/>";
//echo "<a href=\"article.php".substr($last_page->href,3)."&secstr=".$_GET['secstr']."&sec=".$_GET['sec']."\">";
//echo $last_page->innertext."</a>";

//echo " <a href=\"article.php".substr($next_page->href,3)."&secstr=".$_GET['secstr']."&sec=".$_GET['sec']."\">";
//echo $next_page->innertext."</a>";

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
	<?php
	   include('logincheck.php');
	?>
	
	<div class="padbottom">
		<a class="small" href="index.php">首页</a><span class="small">>>阅读信件</span>
	</div>
	
	<div class="section block_head"><?php echo iconv("GB2312","UTF-8//IGNORE",$article_title[0])?></div>
	<?php
		echo "<span class=\"small\">";
		echo iconv("GB2312","UTF-8//IGNORE",$article_author[0]);
		echo "<br/>";
		echo iconv("GB2312","UTF-8//IGNORE",$article_date[0]);
		echo "<br/>-------------<br/>";
		echo "</span>";
		
		$article_content_out=str_replace("<font color=808080>","<span class=\"grey\">",$article_content[0]);
		$article_content_out=str_replace("</font>","</span>",$article_content_out);
		echo iconv("GB2312","UTF-8//IGNORE",$article_content_out);
	?>
	<div class="divider_line"></div>
	
	<div class="small padtop">
	<?php
		echo "<a href='mail.php";
		echo substr($last_page->href,10);
		echo "'>";
		echo iconv("GB2312","UTF-8//IGNORE",$last_page->innertext);
		echo "</a>&nbsp;";
		
		echo "<a href='mail.php";
		echo substr($next_page->href,10);
		echo "'>";
		echo iconv("GB2312","UTF-8//IGNORE",$next_page->innertext);
		echo "</a>";
	 ?>
	</div>
	<div class="small padtop">
		<a href='hmail.php?userid=<?php echo $userfrom[0]."&f=".$_GET['file']."&n=".$_GET['num']."&title=".urlencode(iconv("GB2312","UTF-8//IGNORE",$article_title[0]))?>'>回信</a>
	</div>
	<?php include('footer.php');?>
</body>
</html>
