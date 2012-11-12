<?php 
if($_SESSION["userid"]=="guest")
{	
	$proxy_url ="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsfoot";
	$result = file_get_html($proxy_url);
	$user_online=$result->find('a[href^=bbsufind]',0)->innertext;
}
?>
<div class="divider_line"></div>
<div class="small padtop">
	<form action="alist.php" method="get">
	<input class="textinput" type="text" name="B" size="10" maxlength="20" value="选择讨论区"/>
	<input type="submit" value="GO"/>
	</form>
</div>
<div class="small"><a  href='myboard.php?secstr=*' >我的收藏</a></div>
<div class="small padtop"><a href='maillist.php'>我的信箱<?php echo "<span class=\"red\">".substr($user_mail->innertext,4)."</span>" ?></a></div>
<div class="small padtop"><a  href='friends.php' >查询网友</a></div>
<div class="small padtop"><a  href='index.php' >首页</a></div>
<div class="small padtop"><a  href='about.php' >关于</a></div>
<div class="center small padtop">bmy wap 在线<?php echo $user_online?></div>
