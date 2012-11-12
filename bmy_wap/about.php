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
	<div class="section small">关于wap版BMY</div>
	<div class="small padtop">
	<p>1:wap版是基于web版的</p>
	<p>2:采用XHTML Mobile 1.0协议,少数较老手机不支持</p>
	<p>3:目前是测试阶段,还存在许多问题,欢迎大家提出宝贵意见</p>
	<p>--by xjtusoftware@BMY</p>
	</div>
	<br/>
	<?php include('footer.php');?>
</body>
</html>
