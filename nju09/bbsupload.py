#!/usr/bin/python
import sys,cgi,os,string

usertmpdir="/home/bbsattach/user/"

def er(a):
	print "粘贴失败, 文件太大? 文件不存在? 文件名字包含了特殊字符? 没有登录?<a href=/upload.htm>返回</a>"
	print `a`
	sys.exit(1)

#主程序入口
print "Content-type: text/html; charset=GB2312\n\n\n"
print "<link rel=stylesheet type=text/css href='/bbs.css'>"
if not os.environ.has_key("CONTENT_LENGTH") or string.atoi(os.environ["CONTENT_LENGTH"]) > 5000000:
	er(1)
filesize=string.atoi(os.environ["CONTENT_LENGTH"])
if not os.environ.has_key("PATH_INFO") or len(os.environ["PATH_INFO"])!=34:
	er(2)
sessionid=os.environ["PATH_INFO"][4:]
utmpnum=os.environ["PATH_INFO"][1:4]
fromhost=os.environ["REMOTE_ADDR"]
import re
if not re.match("^\w{3,3}$",utmpnum) or not re.match("^\w{30}$",sessionid):
	er(4)
import commands
userid=commands.getoutput("/usr/local/bin/getuser %s %s %s" % (utmpnum,sessionid,fromhost))
if userid=="test" or userid=="":
	er(5)
form=cgi.FieldStorage()
if not form.has_key("userfile"):
	er(6)
fileitem=form["userfile"]
if fileitem.file:
	filename=string.split(string.replace(fileitem.filename,'\\','/'),'/')[-1]
	if string.strip(filename)=='' or len(filename) > 30:
		er(7)
	for i in filename:
		if i in '$"\',~`:&#@()[]{};><|/\\%!?*\n\r^':
			er(8)
	filename=string.replace(filename,".php3",".html")
	if os.path.exists(usertmpdir+userid):
		nowsize=os.path.getsize(usertmpdir+userid)
	else:
		nowsize=0
	if nowsize+filesize < 5000000 :
		wf=open(usertmpdir+userid , "a")
		import uu
		uu.encode(fileitem.file,wf,filename,0644)
		wf.close()
		print "文件"+filename+"粘贴成功!<a href=/" SMAGIC "%s/bbsupload>点击这里继续粘贴</a>" % os.environ["PATH_INFO"][1:]
	else:
		print "你粘贴太多文件了"
		sys.exit(1)
