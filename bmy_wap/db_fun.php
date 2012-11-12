<?php

class db_log
{    
    public $connect;
    public $select;
 
	function __construct() {
	   include('config_global.php');
       $connect = mysql_connect($global_db_server,$global_db_username,$global_db_pwd);
	   if($connect)
		  { 
	   		$select = mysql_select_db($global_db_db, $connect); 
			mysql_query("SET NAMES 'utf8'");
		  }  
    }
	
	function __destruct() {
    }

	function new_article($action)
	{
		  $userip=GetIP();
		  $action="narticle:".str_replace("\"","@",$action);
		  $action=str_replace("'","*",$action);
		  $sqlstr="INSERT INTO bmyuserlog(username,userip,action,usertime)VALUES ('".$_SESSION["userid"]."','".$userip."','".$action."','".date("Y-m-d H:i:s")."')";
		  mysql_query($sqlstr); 	
	      return true;	 	     
	 }
	 
	function huifu_article($action)
	{
		  $userip=GetIP();
		  $action="harticle:".str_replace("\"","@",$action);
		  $action=str_replace("'","*",$action);
		  
		  $sqlstr="INSERT INTO bmyuserlog(username,userip,action,usertime)VALUES ('".$_SESSION["userid"]."','".$userip."','".$action."','".date("Y-m-d H:i:s")."')";
		  mysql_query($sqlstr); 	
	      return true;	 	     
	 }
	 
	 function save_debug($action)
	{
		  $userip=GetIP();
		  $action="debug:".str_replace("\"","@",$action);
		  $action=str_replace("'","*",$action);
		  $action=iconv("GB2312","UTF-8//IGNORE",$action);
		  $sqlstr="INSERT INTO bmyuserlog(username,userip,action,usertime)VALUES ('".$_SESSION["userid"]."','".$userip."','".$action."','".date("Y-m-d H:i:s")."')";
		  mysql_query($sqlstr); 	
	      return true;	 	     
	 }
	 
	 function save_login()
	{
		  $userip=GetIP();
		  $action="login:";
		  $sqlstr="INSERT INTO bmyuserlog(username,userip,action,usertime)VALUES ('".$_SESSION["userid"]."','".$userip."','".$action."','".date("Y-m-d H:i:s")."')";
		  mysql_query($sqlstr); 	
	      return true;	 	     
	 }
}


?>
