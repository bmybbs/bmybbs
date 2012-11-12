<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

if($_POST['title']=="")
	header("location:narticle.php?board=".$_GET['B']."&e=输入标题");
else
{	
	$postdata="title=".urlencode(iconv("UTF-8","GB2312//IGNORE",$_POST['title']))."&text=".urlencode(iconv("UTF-8","GB2312//IGNORE",$_POST['text'])); 
	$url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/bbssnd?board=".$_GET['B']."&th=-1&signature=1";
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
		  $dbac->new_article($_POST['title']."**".$_POST['text']);
		  header("location:alist.php?B=".$_GET['B']);
		 }
	else
	   {
		 $dbac->save_debug("narticle".$result);
		 header("location:narticle.php?board=".$_GET['B']."&e=未登陆或宵禁");
	   }
}	
?> 
