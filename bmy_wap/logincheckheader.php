<?php
if(isset($PHPSESSID)) 
	session_id($PHPSESSID); 

//local server and bmy server
if(!isset($_SESSION["sessionurl"]))
{
	//get t value
	$sec=explode(" ",microtime());
	$micro=explode(".",$sec[0]);
	$time=$sec[1].substr($micro[1],0,3);
	$proxy_url = "http://bbs.xjtu.edu.cn/BMY/bbslogin?ipmask=8&t=".$time."&id=guest";
	$result = file_get_html($proxy_url);
	$sessionurl=myfind($result,"url=/","/",0);
	if(isset($sessionurl[0]))
	{
		$_SESSION["sessionurl"]=$sessionurl[0];
		$_SESSION["userid"]="guest";
	}
}

?>