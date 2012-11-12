<?php
if($_SESSION['adminuser']=="")
	header("location:login.php");

if($_POST['old']!=""&&$_POST['pwd']!=""&&$_POST['repwd']!="")
{
	include('../config_global.php');
	$connect = mysql_connect($global_db_server,$global_db_username,$global_db_pwd);
	if($connect)
		{ 
			$select = mysql_select_db($global_db_db, $connect); 
			mysql_query("SET NAMES 'utf8'");
			$myuser=$_SESSION['adminuser'];
			if($_POST['pwd']==$_POST['repwd'])
			{
				$mypwd=$_POST['old'];

				$rs=mysql_query("select * from adminuser where username='".$myuser."' and pwd='".$mypwd."'");
				if(mysql_num_rows($rs)>0)
					{
						mysql_query("update adminuser set pwd='".$_POST['repwd']."' where username='".$myuser."'");
						$error="密码修改成功";
					}
				else
				   $error="旧密码输入错误";
			}
			else
				 $error="两次密码输入不一致";
		}
}  
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>兵马俑(wap)后台管理</title>
<link type="text/css" rel="stylesheet" href="extjs/resources/css/ext-all.css">
</head>
<body>
<br>
<table width="100%">
  <tr>
    <td width="5%"></td>
    <td width="90%" align="center">
	<h3 align="center"><a href="index.php">兵马俑wap日志查询系统</a></h3>
	  <br><form action="pwd.php" method="post">
     	旧密码<input name="old" type="text" size="15"/>
		&nbsp;新密码
		<input name="pwd" id="pwd" type="password" size="15"/>
		&nbsp;确认新密码
		<input name="repwd" id="repwd" type="password" size="15"/>
		&nbsp;
		<input type="submit" value="修改" name="bmysearch" id="bmysearch"/>
      </form>
	  <br>
	  <?php echo  $error?>
      <hr width=100% size=1 color=#C8C4C4  align="center">
    </td>
    <td width="5%"></td>
</table>
</body>
</html>
