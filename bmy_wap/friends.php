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
</head>
<link rel="stylesheet" href="css/main.css" type="text/css"/>
<body>
	<?php echo $global_webtop?>
	<br/><br/>
	<div class="padbottom">
		<?php include('logincheck.php');?>
	</div>
	<div class="padtop">
		<form action="friend.php" method="get">
		  <div>
		  	<span class="hint">请输入用户名:</span> <br/>
			<input class="textinput" type="text" name="u" size="10" maxlength="20" value=""/>
			<br/>
			<input type="submit" value="查询网友"/>
		  </div>
		</form>
    </div>
	<?php include('footer.php');?>
</body>
</html>
