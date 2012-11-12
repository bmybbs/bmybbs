<?php

$dbac=new db_log();
$rs=$dbac->out_debug("select * from bmyuserlog");
while($row = mysql_fetch_array($rs))
{
   if(substr($row['action'],0,5)=="debug")
	{
		echo $row[0];
		echo "<br>";
		echo $row['username'];
		echo "<br>";
		echo $row['userip'];
		echo "<br>";
		echo $row['action'];
		echo "<br>";
		echo $row['usertime'];
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		}
}














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
	 
	 function out_debug($sqlstr)
	{
		 return mysql_query($sqlstr); 		     
	 }
	  
}



?>
