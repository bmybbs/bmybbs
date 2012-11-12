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
		<a class="small" href="index.php">首页</a><span class="small">>></span><a class="small" href="alist.php?B=<?php echo $_GET['B']?>"><?php echo $_GET['B']."版" ?></a></div>
	
	<div class="section small">回复文章</div>
	<div class="padbottom">
		<form action="harticle_do.php?B=<?php echo $_GET['B']."&th=".$_GET['th']."&F=".$_GET['F']."&num=".$_GET['num']?>" method="post">
		<div>
			<?php
				if($_GET['e']!="")
					echo "<span class=\"red small\">".$_GET['e']."</span> <br/>";
			?>
			<span class="hint">标题</span> <br/>
			<?php 
				$ttitle=$_GET['t'];
				$title_flag=strpos($ttitle."Re:","Re:");
				if($title_flag>0)
					$ttitle="Re: ".$ttitle;
				//$ttitle=iconv("UTF-8","GB2312//IGNORE",$ttitle)
			?>
			<input class="textinput" type="text" name="title"  value="<?php echo $ttitle ?>"/>
			<br/><span class="hint">正文</span> <br/>
			<textarea name="text" rows="4"></textarea>
			<br>
			<input type="submit" value="发表"/>
		</div>
		</form>
	</div>
	<?php include('footer.php');?>
</body>
</html>
