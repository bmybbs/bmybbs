<?php

if(isset($PHPSESSID)) 
	session_id($PHPSESSID); 

if($_GET['action']=="logout"&&isset($_SESSION["sessionurl"]))
{
	$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbslogout";
	$result = file_get_html($proxy_url);
	unset($_SESSION["sessionurl"]);
	unset($_SESSION["userid"]);
	session_destroy(); 
	setcookie('PHPSESSID', "");  
	header("location:index.php");
}

echo "<div class=\"section small\">";

if($_POST['id']&&$_SESSION["userid"]=="guest")
{	
	//get t value
	$sec=explode(" ",microtime());
	$micro=explode(".",$sec[0]);
	$time=$sec[1].substr($micro[1],0,3);
	$proxy_url = "http://bbs.xjtu.edu.cn/BMY/bbslogin?ipmask=8&t=".$time."&id=".$_POST['id']."&pw=".$_POST['pw'];
	$result = file_get_html($proxy_url);
	
	//echo $result;
	if(strstr($result,iconv("UTF-8","GB2312//IGNORE","错误! 密码错误!"))||strstr($result,iconv("UTF-8","GB2312//IGNORE","错误! 错误的使用者帐号!")))
	{
		echo "错误! 账号或密码错误!";
		unset($_SESSION["sessionurl"]);
		unset($_SESSION["userid"]);
	}
	else
	{
		if(strstr($result,iconv("UTF-8","GB2312//IGNORE","错误! 两次登录间隔过密!!")))
		{
			echo "错误! 两次登录间隔过密!";
			unset($_SESSION["sessionurl"]);
			unset($_SESSION["userid"]);
		}
		else
		{
			$sessionurl_t=myfind($result,"url=/","/",0);
			if(isset($sessionurl_t[0]))
			{
				$_SESSION["sessionurl"]=$sessionurl_t[0];
				$_SESSION["userid"]=$_POST['id'];
				$_SESSION["autologin"]=$_POST['autologin'];
				$_SESSION["password"]=$_POST['pw'];
				
				$PHPSESSID = session_id();  
				if($_POST['autologin']=="true")
				{  
					setcookie('PHPSESSID', $PHPSESSID, time()+12*2592000);  
				}
				else
				{
					setcookie('PHPSESSID',$PHPSESSID);  
				}
				$dbac=new db_log();
				$dbac->save_login();
			}
		}
	}
	//登陆
	//转向
	//写入session  bmy session url
	//$_GET["url"]; 
}

if(isset($_SESSION["sessionurl"]))
{	
	
	if($_SESSION["userid"]!="guest")
	   { 
		  $proxy_url2 ="http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbsfoot";
		  $result2 = file_get_html($proxy_url2);
		  $user_name=$result2->find('a[href^=bbsqry]',0);
		  
		  if($user_name->innertext=="guest")
			{
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
			    $result2 = file_get_html($proxy_url);
			}	
		    $user_name=$result2->find('a[href^=bbsqry]',0);
			if($user_name->innertext=="guest")
				header("location:index.php?action=logout");
			
			echo $_SESSION["userid"];
			$user_online=$result2->find('a[href^=bbsufind]',0)->innertext;
		    $user_mail=$result2->find('a[href^=bbsmail] font',0);
		    echo "<a href='index.php?action=logout' class=\"small padtop\">注销</a>";
			if($_SESSION["userid"]!="guest")
			{
				echo "|<a href='maillist.php'>信箱<span class=\"red\">".substr($user_mail->innertext,4)."</span></a>";
				echo "|<a href='myboard.php?secstr=*' >收藏夹</a>";
			}
				
	   }
	 else
	 {
	       echo $_SESSION["userid"]; 
	 }
}
else
{
	//get t value
	$sec=explode(" ",microtime());
	$micro=explode(".",$sec[0]);
	$time=$sec[1].substr($micro[1],0,3);
	$proxy_url = "http://bbs.xjtu.edu.cn/BMY/bbslogin?ipmask=8&t=".$time."&id=guest";
	$result = file_get_html($proxy_url);
	$sessionurl_t=myfind($result,"url=/","/",0);
	if(isset($sessionurl_t[0]))
	{
		$_SESSION["sessionurl"]=$sessionurl_t[0];
		$_SESSION["userid"]="guest";
	}
}

echo "</div>";

if($_SESSION["userid"]=="guest")
{
?>
	<form action="index.php" method="post">
		<div>
			<span class="hint">账号</span> <br/>
			<input class="textinput" type="text" name="id" maxlength="20" value="" size="12"/>
			<br/><span class="hint">密码</span> <br/>
			<input class="textinput" type="password" name="pw" maxlength="20" value="" size="12"/>
			<br><input name="autologin" type="checkbox" value="true" checked="true"/><span class="hint">下次自动登录</span>
			<br/><input type="submit" value="登录"/>
		</div>
	</form>
<?php
}
//echo "<br>";
//echo $_SESSION["sessionurl"];
//echo "<br>";
//echo $_SESSION["userid"];
?>