<div class="small">
<?php

//local server and bmy server
if(isset($_SESSION["sessionurl"]))
{	
	if($_SESSION["userid"]!="guest")
	{
		
		//check bmy server
		$proxy_url ="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsfoot";
		$result = file_get_html($proxy_url);
		$user_name=$result->find('a[href^=bbsqry]',0);
		//echo $user_name;
		if($user_name->innertext=="guest")
		{	
			//get t value
			$sec=explode(" ",microtime());
			$micro=explode(".",$sec[0]);
			$time=$sec[1].substr($micro[1],0,3);
			$proxy_url = "http://bbs.xjtu.edu.cn/BMY/bbslogin?ipmask=8&t=".$time."&id=".$_SESSION["userid"]."&pw=".$_SESSION["password"];
			$result_x = file_get_html($proxy_url);
			$sessionurl_t=myfind($result_x,"url=/","/",0);
			if(isset($sessionurl_t[0]))
			{
				$_SESSION["sessionurl"]=$sessionurl_t[0];
				$dbac=new db_log();
				$dbac->save_login();
			}
			else
				header("location:index.php?action=logout");

			$proxy_url ="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsfoot";
			$result = file_get_html($proxy_url);
		}
		
		$user_name=$result->find('a[href^=bbsqry]',0);
		if($user_name->innertext=="guest")
			header("location:index.php?action=logout");
			
		$user_online=$result->find('a[href^=bbsufind]',0)->innertext;
		$user_mail=$result->find('a[href^=bbsmail] font',0);
		echo $_SESSION["userid"];
		echo "<a href='index.php?action=logout' class=\"small padtop\">注销</a>";
		
	}
	else
	{	
	   echo "guest";
	   echo "<a href='index.php' class=\"small padtop\">登录</a>";
	}
}

if($_SESSION["userid"]!="guest")
{
	echo "|<a href='maillist.php'>信箱<span class=\"red\">".substr($user_mail->innertext,4)."</span></a>";
	echo "|<a href='myboard.php?secstr=*' >收藏夹</a>";
}
?>
</div>
<div class="divider_line"></div>