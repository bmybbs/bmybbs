<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

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
		<a class="small" href="index.php">首页</a><span class="small">>>发送信件</span></div>
	
	<div class="section small">发送信件</div>
	<div class="padbottom">
		<form action="nmail_do.php?<?php echo "f=".$_GET['f']."&n=".$_GET['n'] ?>>" method="post">
		<div>
			<?php
				if($_GET['e']!="")
					echo "<span class=\"red small\">".$_GET['e']."</span> <br/>";
				
				$ttitle=$_GET['title'];
				$title_flag=strpos($ttitle."Re:","Re:");
				if($title_flag>0)
					$ttitle="Re: ".$ttitle;
			?>
			<span class="hint">标题</span> <br/>
			<input class="textinput" type="text" name="title" value="<?php echo $ttitle?>"/>
			<br/><span class="hint">收信人</span> <br/>
			<input class="textinput" type="text" name="userid" maxlength="20" value="<?php echo $_GET['userid']?>"/>
			<br/><span class="hint">正文</span> <br/>
			<textarea name="text" rows="4"></textarea>
			<br>
			<input type="submit" value="提交"/>
		</div>
		</form>
	</div>
	<?php include('footer.php');?>
</body>
</html>
