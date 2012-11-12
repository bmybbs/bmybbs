<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('configindex.php');

?>
<!DOCTYPE html PUBLIC "-//WAPFORUM//DTD XHTML Mobile 1.0//EN" "http://www.wapforum.org/DTD/xhtml-mobile10.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title><?php echo $global_webtitle?></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head>
</head>
<link rel="stylesheet" href="css/main.css" type="text/css"/>
<body>
	<?php echo $global_webtop?>
	<br/><br/>
	<div class="padbottom">
		<?php include('loginindex.php');?>
		<form action="alist.php" method="get">
		  <div>
			<input class="textinput" type="text" name="B" size="10" maxlength="20" value="选择讨论区"/>
			<input type="submit" value="GO"/>
		  </div>
		</form>
    </div>
	<div class="section small">十大热门话题</div>
	<?php
		$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbstop10";
		$result = file_get_html($proxy_url);
		
		//get shida content
		$sd_list=$result->find('a[href^=bbstfind]');
		foreach($sd_list as $ttoffset=>$tt) 
		{ 
			$tt->href="talist.php?B=".str_replace("&title","&T",substr($tt->href,15));
			$tt->href=iconv("GB2312","UTF-8//IGNORE",$tt->href);
			$tt->innertext=trim(iconv("GB2312","UTF-8//IGNORE",$tt->innertext));
			//if(strlen($tt->innertext)>50)
			//	$tt->innertext=substr($tt->innertext,0,50);
			echo "<div class=\"small padtop\">".($ttoffset+1).$tt."</div>";
		}
	?>
	
	<br/>
	<div class="section small"> 分类讨论区 </div>
	<div class="small padtop"> <a  href='board.php?secstr=0' >本站系统 </a> | <a  href='board.php?secstr=1' >交通大学 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=2' >开发技术 </a> | <a  href='board.php?secstr=3' >电脑应用 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=4' >学术科学 </a> | <a  href='board.php?secstr=5' >社会科学 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=6' >文学艺术 </a> | <a  href='board.php?secstr=7' >知性感性 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=8' >体育运动 </a> | <a  href='board.php?secstr=9' >休闲音乐 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=G' >游戏天地 </a> | <a  href='board.php?secstr=N' >新闻信息 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=H' >乡音乡情 </a> | <a  href='board.php?secstr=A' >校务信息 </a></div>
	<div class="small padtop"> <a  href='board.php?secstr=C' >俱乐部区 </a><br/></div>

	<?php include('footer.php');?>
</body>
</html>
