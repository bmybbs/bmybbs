#!/usr/bin/python
import  cPickle,time,sys,string
f=open("binlog","r")
log=cPickle.load(f)
f.close()
if(len(sys.argv)<2):
	disk=""
else:
	disk="/dev/ythtvg/"+sys.argv[1]
nday=log[0]["time"]
rc=0
wc=0
for i in range(0,len(log)):
	cday=log[i]["time"]
        if(cday[0:3]==nday[0:3]):
		if(cday[3]>9 and cday[3]<23):
			if disk!="":
				if log[i]["dev"].has_key(disk):
					rc=rc+log[i]["dev"][disk]["read"]
					wc=wc+log[i]["dev"][disk]["write"]
			else:
				for j in log[i]["dev"].keys():
					rc=rc+log[i]["dev"][j]["read"]
					wc=wc+log[i]["dev"][j]["write"]
	else:
		print "%d %d %d" % nday[0:3]
		print disk,rc,wc,rc+wc
		nday=cday
		rc=0
		wc=0
print "%d %d %d" % nday[0:3]
print disk,rc,wc,rc+wc
	
