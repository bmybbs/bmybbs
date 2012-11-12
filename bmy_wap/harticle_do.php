<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

if($_POST['title']=="")
	header("location:harticle.php?B=".$_GET['B']."&e=输入标题");
else
{	
	$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/pst?B=".$_GET['B']."&F=".$_GET['F']."&num=".$_GET['num'];
    $result = file_get_html($proxy_url);
	$article_ref=$result->find('textarea[name=text]',0);
	
	$ttitle=$_POST['title'];
	if(!strstr($ttitle,"Re:"))
		$ttitle="Re: ".$ttitle;
	$postdata="title=".urlencode(iconv("UTF-8","GB2312//IGNORE",$ttitle))."&text=".urlencode(iconv("UTF-8","GB2312//IGNORE",$_POST['text']).$article_ref->innertext); 
	$url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbssnd?board=".$_GET['B']."&th=".$_GET['th']."&ref=".$_GET['F']."&rid=".$_GET['num'];
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	curl_setopt($ch, CURLOPT_URL,$url);    
	curl_setopt($ch, CURLOPT_POSTFIELDS, $postdata);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
	$result = curl_exec($ch);
	curl_close($ch);
	
	$dbac=new db_log();
	if(strstr($result,"bbsdoc")&&strstr($result,"Refresh"))
		{
		   $dbac->huifu_article($ttitle."**".$_POST['text']);
		   header("location:alist.php?B=".$_GET['B']);
		}
	else
		{
		  $dbac->save_debug("harticle".$result);
		  header("location:harticle.php?B=".$_GET['B']."&e=未登陆或宵禁");  
		}
}	
?> 

