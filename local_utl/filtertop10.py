#!/usr/bin/python
import os,string
top10="/home/bbs/etc/posts/day"
f=open(top10,"r")
con=f.readlines()
f.close()
filter="/home/bbs/etc/filtertitle"
f=open(filter,"r")
filter=f.read(os.path.getsize(filter))
f.close()
if len(con)!=22:
	os.exit(1)
flist=string.split(filter,"\n")
for i in range(10):
	has=0
	for j in range(len(flist)):
		if flist[j]!="" and ( string.find(con[i*2+2],flist[j])!=-1 or string.find(con[i*2+3],flist[j])!=-1):
			has=1
			break
	
	if has==1:
		con[i*2+3]="     \033[37m标题 : \033[1;44;37m可能引起不快的内容,不适合在公众场合展示                     \033[40m\n"
for i in range(len(con)):
	print con[i],
