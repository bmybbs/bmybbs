<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/con?B=".$_GET['B']."&F=".$_GET['F']."&N=".$_GET['N']."&T=".$_GET['T'];
$result = file_get_html($proxy_url);

//header parameters
$header_para=$result->find('a');
$header_flag=0;
if(strstr($header_para[0]->innertext,iconv("UTF-8","GB2312//IGNORE","看不了图片")))
	$header_flag=1;

//the same article lsit
$tzt_para=$result->find('a[href^=tfind]',0);

//gete all the data needed
$article_all=$result->find('.bordertheme',0)->innertext;
$article_all=str_replace("<br>","<br/>",$article_all);
$article_all=substr($article_all,0,strlen($article_all)-18);
$article_title=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","标 &nbsp;题: "),"<br/>",0);
$article_content=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","本站(bbs.xjtu.edu.cn)"),"<br/>--",0);
$article_date=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","发信站: 兵马俑BBS ("),")",0);
$article_author=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","发信人: "),",",0);
$article_author_id=myfind("$$$".$article_author[0],"$$$"," (",0);
if(strstr($result,iconv("UTF-8","GB2312//IGNORE","回复本文")))
	$huifu_flag=true;
else
	$huifu_flag=false;
//heji
if(strstr($article_title[0],iconv("UTF-8","GB2312//IGNORE","【合集】")))
{

	$article_content=myfind($article_all."**##",iconv("UTF-8","GB2312//IGNORE","本站(bbs.xjtu.edu.cn)"),"**##",0);
	$article_content[0]=str_replace("<font class=h32","<span class=\"green small\"",$article_content[0]);
	$article_content[0]=str_replace(iconv("UTF-8","GB2312//IGNORE","──────────────────────────────────────"),iconv("UTF-8","GB2312//IGNORE","───────"),$article_content[0]);
	$article_content[0]=str_replace("<font class=b40","<span class=\"small\"",$article_content[0]);
	$article_content[0]=str_replace("<font class=b40","<span class=\"small\"",$article_content[0]);
	$article_content[0]=str_replace("<font class=c37","<span class=\"small\"",$article_content[0]);
	$article_content[0]=str_replace("<br/>&nbsp; &nbsp; ","<br/>",$article_content[0]);

}

//change the pics
$article_pics=myfind($article_content[0],"<img src='","'>",0);
//echo "<img src='".$article_pics[0]."'>";

//$t = new ThumbHandler();

$cm=new CreatMiniature();
foreach($article_pics as $article_pic)
{	
 	if($article_pic!="/images/spacer.gif")
	{		 	
		srand((double)microtime()*1000000); 
		$randval = rand(0,999); 
		
		$cm->SetVar($article_pic,"file");
		$new_pic_name=time().$randval.".jpg";
		@$cm->Prorate("tempimg/".$new_pic_name,150,200);
		if(strstr($article_pic,"&amp;")&&strstr($article_pic,"attachname="))
			$article_pic_t="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/".str_replace("&amp;","&",$article_pic);
		else
			$article_pic_t=$article_pic;
		$article_content[0]=str_replace($article_pic_t,"tempimg/".$new_pic_name,$article_content[0]);
		//article_pic="tempimg/".$new_pic_name;
	}
}

//output footer
$last_page=$result->find('a[accesskey=f]',0);
$next_page=$result->find('a[accesskey=n]',0);

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
	<?php include('logincheck.php');?>
	<div class="padbottom">
		<a class="small" href="index.php">首页</a><span class="small">>></span><a class="small" href="<?php echo "board.php".substr($header_para[$header_flag]->href,3)?>"><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[$header_flag]->innertext) ?></a><span class="small">>></span><a class="small" href="<?php echo "alist.php".substr($header_para[$header_flag+1]->href,4)?>"><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[$header_flag+1]->innertext) ?></a>
	</div>
	
	<div class="section block_head"><?php echo iconv("GB2312","UTF-8//IGNORE",$article_title[0])?></div>
	<?php
		echo "<span class=\"small\">";
		echo "<a href='friend.php?u=".$article_author_id[0]."'>".iconv("GB2312","UTF-8//IGNORE",$article_author[0])."</a>";
		echo "<br/>";
		echo iconv("GB2312","UTF-8//IGNORE",$article_date[0]);
		echo "<br/>-------------";
		echo "</span>";
		
		$article_content_out=str_replace("<font color=808080>","<span class=\"grey\">",substr($article_content[0],7));
		$article_content_out=str_replace("</font>","</span>",$article_content_out);
		echo iconv("GB2312","UTF-8//IGNORE",$article_content_out);
	?>
	<div class="divider_line"></div>
	
	<div class="small padtop">
	<?php
		echo "<a href='article.php";
		echo substr($last_page->href,3);
		echo "'>";
		echo iconv("GB2312","UTF-8//IGNORE",substr($last_page->innertext,0,strlen($last_page->innertext)-1));
		echo "</a>&nbsp;";
		
		echo "<a href='article.php";
		echo substr($next_page->href,3);
		echo "'>";
		echo iconv("GB2312","UTF-8//IGNORE",$next_page->innertext);
		echo "</a>";
	 ?>
	</div>
	<div class="small padtop">
		<a href='alist.php?B=<?php echo $_GET['B']."&S=".$_GET['S']?>'>返回讨论区</a><br/>
	</div>
	<div class="small padtop">
	<?php
		$tzt_para->href=iconv("GB2312","UTF-8//IGNORE","talist.php".substr($tzt_para->href,5));
		$tzt_para->innertext=iconv("GB2312","UTF-8//IGNORE",substr($tzt_para->innertext,2));
		echo "<a href='";
		echo $tzt_para->href;
		echo "'>";
		echo $tzt_para->innertext;
		echo "</a>";
		$th=myfind($tzt_para->href,"th","&",0);
	?>
	</div>
	<?php
	    if($huifu_flag)
		{
			echo "<div class=\"small padtop\">";
			echo  "<a href='harticle.php?B=".$_GET['B']."&F=".$_GET['F']."&num=".($_GET['N']-1)."&th=".$th[0]."&t=".urlencode(iconv("GB2312","UTF-8//IGNORE",$article_title[0]))."'>回复本文</a><br/>";
			echo "</div>";
		}
	?>
	<div class="small padtop">
		<a href='nmail.php?touser=<?php echo $article_author_id[0]."&t=".urlencode(iconv("GB2312","UTF-8//IGNORE",$article_title[0]))?>'>回信给作者</a><br/>
	</div>
	<?php
		include('footer.php');
	?>
</body>
</html>
