#!/usr/bin/python
import sys
import re
maillog="/home/bbs/mail-log"
f=open(maillog,"r")
mailer={}
a=f.readline()
fff=re.compile("^From (\S+) ")
ttt=re.compile("^To: (\S+)\.bbs")
sss=re.compile("^Subject: (.*)")
sender={}
while a!="":
	t=fff.match(a)
	if(t!=None):
		if mailer.has_key("to") and mailer.has_key("from") and mailer.has_key("sub"):
			if not sender.has_key(mailer["from"]):
				sender[mailer["from"]]={}
			if not sender[mailer["from"]].has_key(mailer["sub"]):
				sender[mailer["from"]][mailer["sub"]]={}
			sender[mailer["from"]][mailer["sub"]][mailer["to"]]=1
		mailer={}
		mailer["from"]=t.group(1)
	else:
		t=ttt.match(a)
		if(t!=None):
			mailer["to"]=t.group(1)
		else:
			t=sss.match(a)
			if(t!=None):
				mailer["sub"]=t.group(1)
	a=f.readline()
for i in sender.keys():
	for j in sender[i].keys():
		sendtime=len(sender[i][j])
		if sendtime > 2:
			print i,j,sendtime
