<?php
	echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
?>
<?php
include('config.php');

//get urlcontent
$proxy_url = "http://bbs.xjtu.edu.cn/".$_SESSION["sessionurl"]."/boa?secstr=".$_GET['secstr'];
$result = file_get_html($proxy_url);

//get the board list
//$board_list=$result->find('a[href^=bbsboa],a[href^=home],a[href^=tdoc]');
$board_list=$result->find('td[class=tdborder]');

//get the uper board name
$up_board_name=$result->find('a[class=btnsubmittheme]',0);
$up_board_name->innertext=iconv("GB2312","UTF-8//IGNORE",$up_board_name->innertext);
//$up_board_name->href="board.php".substr($up_board_name->href,3);
//echo $up_board_name->innertext;

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
		<a class="small" href="index.php">首页</a><span class="small">>></span><span class="small"><?php echo $up_board_name->innertext?></span>
	</div>
	<div class="section small">讨论区</div>
	<?php
		foreach($board_list as $ttoffset=>$templist) 
		{ 
			if($ttoffset%8==0)
			{	
			    if($board_list[$ttoffset]->innertext==iconv('UTF-8', 'gb2312',"◆"))
					$unread="*";
				else
					$unread="";
			
				$tt=$board_list[$ttoffset+3]->find('a',0);
				$listtext_lefter="(".substr($tt->href,7).")";
				
				if (substr($tt->href,0,6)=="bbsboa")
				{
					$tt->href="board.php".substr($tt->href,6);
					$listtext_lefter="二级版面";
				}
			    else
					$tt->href="alist.php".substr($tt->href,4);
				$tt->innertext=iconv("GB2312","UTF-8//IGNORE",$tt->innertext).$listtext_lefter;
				
				//echo $tt."<br>";
				echo "<div class=\"small padtop\">".$unread.$tt.$board_list[$ttoffset+7]->innertext."</div>";
			}
		}

	?>
	<?php include('footer.php');?>
</body>
</html>
