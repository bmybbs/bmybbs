<?php
if($_SESSION['adminuser']=="")
	header("location:login.php");



$sqlstr="select * from bmyuserlog where id>0 ";
if($_GET['id']!="")
	$sqlstr=$sqlstr." and username='".$_GET['id']."' ";
	
if($_GET['ip']!="")
	$sqlstr=$sqlstr." and userip='".$_GET['ip']."' ";

if($_GET['starttime']!="")
	$sqlstr=$sqlstr." and usertime>='".$_GET['starttime']."' ";

if($_GET['stoptime']!="")
	$sqlstr=$sqlstr." and usertime<='".$_GET['stoptime']."' ";
	
if($_GET['content']!="")
	$sqlstr=$sqlstr." and action like'%".iconv("GB2312","UTF-8//IGNORE",$_GET['content'])."%' ";

$sqlstr=$sqlstr." and action like '%article%' order by id desc limit 0,50";

$dbac=new db_log();
$rs=$dbac->out_debug($sqlstr);

$offset=0;
$offsetflg=0;
$result="total:'".mysql_num_rows($rs)."',bmy:[";

while($row = mysql_fetch_array($rs))
{		
		if($offset>=$_GET['start']&&$offsetflg<50)
		{ 
			$temp="{";
			$temp=$temp."id:'".$row['username']."',";
			$temp=$temp."ip:'".$row['userip']."',";
			$temp=$temp."time:'".$row['usertime']."',";
			
			/*
			if(strpos($row['action'],"harticle")==0)
				$actiontype=iconv("GB2312","UTF-8//IGNORE","»Ø¸´");
			if(strpos($row['action'],"narticle")==0)
				$actiontype=iconv("GB2312","UTF-8//IGNORE","·¢±í");
			if(strpos($row['action'],"login")==0)
				$actiontype=iconv("GB2312","UTF-8//IGNORE","µÇÂ½");
			*/
			if(strpos($row['action'],"login:")==0)
				{
					$actionout=substr($row['action'],strpos($row['action'],":")+1);
					$content=substr($actionout,strpos($actionout,"**")+2);
					$title=substr($actionout,0,strlen($actionout)-strlen($content)-2);
					$title=str_replace(iconv("GB2312","UTF-8//IGNORE","£¿"),"",$title);
				}
				else
					$title="";
			
			$temp=$temp."type:'".$actiontype."',";
			$temp=$temp."title:'".$title."'}";
			
			if ($offset==0)
				{
				$result=$result.$temp;
				}
			else 
				{ 
				$result=$result.",".$temp;
				}
				
			$offsetflg++;	
		}
		$offset++;
					
}
echo "{".$result."]}";
//functions

class db_log
{    
    public $connect;
    public $select;
 
	function __construct() {
	   include('../config_global.php');
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
