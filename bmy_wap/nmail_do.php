<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php

include('config.php');

if($_POST['title']==""||$_POST['userid']=="")
	header("location:nmail.php");
else
{	
	if($_GET['f']!="")
	{
		$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbspstmail?file=".$_GET['f']."&num=".$_GET['n'];
    	$result = file_get_html($proxy_url);
		$mail_ref=$result->find('textarea[name=text]',0);
		$mail_ref_out=$mail_ref->innertext;
	}
	$ttitle=$_POST['title'];
	$postdata="title=".urlencode(iconv("UTF-8","GB2312//IGNORE",$ttitle))."&text=".urlencode(iconv("UTF-8","GB2312//IGNORE",$_POST['text']).$mail_ref_out)."&userid=".urlencode(iconv("UTF-8","GB2312//IGNORE",$_POST['userid'])); ; 
	$url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbssndmail";
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch, CURLOPT_URL,$url);    
	curl_setopt($ch, CURLOPT_POSTFIELDS, $postdata);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
	$result = curl_exec($ch);
	curl_close($ch);
	
	$dbac=new db_log();
	if(strstr($result,iconv("UTF-8","GB2312//IGNORE","信件已寄给")))
		{
			header("location:nmail.php?e=发送成功");
		}
	else
		{
		    $dbac->save_debug("mail".$result);
			header("location:nmail.php?e=发送失败");
		}
}	
?> 
