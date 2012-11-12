<?php
if($_SESSION['adminuser']=="")
	header("location:login.php");

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>兵马俑(wap)后台管理</title>
<link type="text/css" rel="stylesheet" href="extjs/resources/css/ext-all.css">
<script type="text/javascript" src="extjs/adapter/ext/ext-base.js"></script>
<script type="text/javascript" src="extjs/ext-all.js"></script>
<script type="text/javascript" src="bmy.js"></script>
<style type="text/css">
<!--
.STYLE2 {
font-size: 12px
}
-->
</style>
</head>
<body>
<br>
<table width="100%">
  <tr>
    <td width="5%"></td>
    <td width="90%" align="center">
	 <h3 align="center">兵马俑wap日志查询系统<a href="login.php?action=logout">注销</a>&nbsp;&nbsp;<a href="pwd.php">修改密码</a></h3>
	  <br>
     	用户名<input name="user" id="user"  type="text" size="10"/>
		&nbsp;ip<input name="ip" id="ip" type="text" size="15"/>
		&nbsp;开始时间
		<input name="starttime" id="starttime" type="text" size="10"/>
	  &nbsp;结束时间
		<input name="stoptime"  id="stoptime"  type="text" size="10"/>
		&nbsp;标题内容<input name="content"  id="content"  type="text" size="15" />
		&nbsp;<input type="submit" value="检索" name="bmysearch" id="bmysearch"/>
  
      <hr width=100% size=1 color=#C8C4C4  align="center">
	  <table>
	  	<tr>
		 <td align="center" width="100%">
		  <div id="bmy" align="left" style="width:100%" > <br>
			<br><br><br><br><br><br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
			<br>
		  </div>		 </td>
		</tr>
	  </table>
      <hr width=100% size=1 color=#C8C4C4  align="center">    </td>
    <td width="5%"></td>
</table>
</body>
</html>
