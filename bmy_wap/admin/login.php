<?php
if($_GET['action']=="logout")
{
	unset($_SESSION["adminuser"]);
	session_destroy(); 
	header("location:login.php");
}

if($_POST['user']!=""&&$_POST['pwd']!="")
{
	include('../config_global.php');
	$connect = mysql_connect($global_db_server,$global_db_username,$global_db_pwd);
	if($connect)
		{ 
			$select = mysql_select_db($global_db_db, $connect); 
			mysql_query("SET NAMES 'utf8'");
			$myuser=str_replace(" ","",$_POST['user']);
			$myuser=str_replace("'","",$myuser);
			$myuser=str_replace("\"","",$myuser);
			
			$mypwd=str_replace(" ","",$_POST['pwd']);
			$mypwd=str_replace("'","",$mypwd);
			$mypwd=str_replace("\"","",$mypwd);
			$rs=mysql_query("select * from adminuser where username='".$myuser."' and pwd='".$mypwd."'");
			if(mysql_num_rows($rs)>0)
				{
					$_SESSION['adminuser']=$myuser;	
					header("location:index.php");
				}
			else
			   $error="用户名或密码错误";
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
	 <h3 align="center">&nbsp;</h3>
	  <br><form action="login.php" method="post">
     	用户名<input name="user" id="user"  type="text" size="15"/>
		&nbsp;密码
		<input name="pwd" id="pwd" type="password" size="15"/>
		&nbsp;
		<input type="submit" value="登陆" name="bmysearch" id="bmysearch"/>
      </form>
	  <br>
	  <?php echo  $error?>
      <hr width=100% size=1 color=#C8C4C4  align="center">
    </td>
    <td width="5%"></td>
</table>
</body>
</html>
