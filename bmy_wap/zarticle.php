<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbstcon?board=".$_GET['board']."&start=".$_GET['start']."&th=".$_GET['th'];
$result = file_get_html($proxy_url);
/*
//pics
$article_pics=$result->find('img[src]');
$cm=new CreatMiniature();
foreach($article_pics as $article_pic)
{	
 	if($article_pic->src!="/images/spacer.gif")
	{
		$cm->SetVar($article_pic->src,"file");
		$new_pic_name=time().".jpg";
		$cm->Prorate("tempimg/".$new_pic_name,150,200);
		$article_pic->src="tempimg/".$new_pic_name;
	}
}
*/
//output content
$article_alls=$result->find('td[class=bordertheme],td[class=bordergrey]');

//header parameters
$header_para=$result->find('a');

//output footer
$articles_pages=$result->find('a');
$page_flag=0;
$page_flag2=0;
$page_flag3=0;
?> 
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.0//EN" "http://www.wapforum.org/DTD/xhtml-mobile10.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title><?php echo $global_webtitle?></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>
<link rel="stylesheet" href="css/main.css" type="text/css"/>
<body>
	<?php echo $global_webtop?>
	<br/>
	<br/>
	<?php include('logincheck.php');?>
	<div class="padbottom">
		<a class="small" href="index.php">首页</a><span class="small">>></span><a class="small" href="<?php echo "board.php".substr($header_para[0]->href,3)?>"><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[0]->innertext) ?></a><span class="small">>></span><a class="small" href="<?php echo "zalist.php?board=".substr($header_para[1]->href,7)?>"><?php echo iconv("GB2312","UTF-8//IGNORE",$header_para[1]->innertext) ?></a>	</div>
	
	<?php
		foreach($article_alls as $aoffset=>$article_temp)
		{	
			$article_all=$article_temp->innertext;
			$article_all=str_replace("<br>","<br/>",$article_all);
			$article_all=substr($article_all,0,strlen($article_all)-94);
			$article_title=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","标 &nbsp;题: "),"<br/>",0);
			$article_content=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","本站(bbs.xjtu.edu.cn)"),"<br/>--",0);
			$article_date=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","发信站: 兵马俑BBS ("),")",0);
			$article_author=myfind($article_all,iconv("UTF-8","GB2312//IGNORE","发信人: "),",",0);
			$article_author_id=myfind("$$$".$article_author[0],"$$$"," (",0);
			
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
					$article_content[0]=str_replace($article_pic,"tempimg/".$new_pic_name,$article_content[0]);
				}
			}

			
			
			echo "<div class=\"section block_head\">";
			echo iconv("GB2312","UTF-8//IGNORE",$article_title[0]);
			echo "</div>";
			
			echo "<span class=\"small\">";
			echo "<a href='friend.php?u=".$article_author_id[0]."'>".iconv("GB2312","UTF-8//IGNORE",$article_author[0])."</a>";
			echo "<br/>";
			echo iconv("GB2312","UTF-8//IGNORE",$article_date[0]);
			echo "<br>-------------";
			echo "</span>";
			
			$article_content_out=str_replace("<font color=808080>","<span class=\"grey\">",substr($article_content[0],7));
			$article_content_out=str_replace("</font>","</span>",$article_content_out);
			echo iconv("GB2312","UTF-8//IGNORE",$article_content_out);
		}
	?>
	<div class="divider_line"></div>
	
	<div class="small padtop">
	<?php
		foreach($articles_pages as $tt) 
		{   
			   if($tt->innertext==iconv("UTF-8","GB2312//IGNORE","首页")&&$page_flag==0)
			   {
				echo "<a href=\"zarticle.php".substr($tt->href,7)."\">";
				echo "第一页"."</a>";
				$page_flag=1;
			   }
			   if($tt->innertext==iconv("UTF-8","GB2312//IGNORE","下页")&&$page_flag2==0)
			   {
				echo "|<a href=\"zarticle.php".substr($tt->href,7)."\">";
				echo iconv("GB2312","UTF-8//IGNORE",$tt->innertext)."</a>";
				$page_flag2=1;
			   }
			   if($tt->innertext==iconv("UTF-8","GB2312//IGNORE","尾页")&&$page_flag3==0)
			   {
				echo "|<a href=\"zarticle.php".substr($tt->href,7)."\">";
				echo iconv("GB2312","UTF-8//IGNORE",$tt->innertext)."</a>";
				$page_flag3=1;
			   }
		}
	 ?>
	</div>
	<div class="small padtop">
		<a href='zalist.php?board=<?php echo $_GET['board']."&S=".$_GET['S']?>'>返回讨论区</a><br/>
	</div>
	<?php include('footer.php');?>
</body>
</html>
