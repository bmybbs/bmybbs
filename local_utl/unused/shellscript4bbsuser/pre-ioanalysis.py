#!/usr/bin/python
import sys,re,time,string,cPickle

if (len(sys.argv)<2):
	print "error,no enough argments!"
	sys.exit(0)
f=open(sys.argv[1],"r")
a=f.readlines()
f.close()
log=[]
t=re.compile("((\w{3,3} ){2,2}\s?\d{1,2} (\d{2,2}:){2,2}\d{2,2} \d{4,4})")
d=re.compile("Logical volume \"(.*)\"")
c=re.compile("Total reads:\s+(\d+)\s+Total writes:\s+(\d+)")
item={}
for i in a:
	r=t.match(i)
	if r !=None :
		if (len(item)>0):
			log.append(item)		
		item={"time":time.strptime(r.groups()[0]),"dev":{}}
		continue
	r=d.match(i)
	if r !=None :
		tk=r.groups()[0]
		item["dev"][tk]={}
		continue
	r=c.match(i)
	if r !=None :
		item["dev"][tk]["read"]=string.atoi(r.groups()[0])
		item["dev"][tk]["write"]=string.atoi(r.groups()[1])
for i in range(len(log)-1,0,-1):
	for j in log[i]["dev"].keys():
		if (log[i-1]["dev"].has_key(j)):
			tmp=log[i]["dev"][j]["read"]-log[i-1]["dev"][j]["read"]
		else:
			tmp=log[i]["dev"][j]["read"]
		if (tmp >= 0):
			log[i]["dev"][j]["read"]=tmp
		if (log[i-1]["dev"].has_key(j)):
			tmp=log[i]["dev"][j]["write"]-log[i-1]["dev"][j]["write"]
		else:
			tmp=log[i]["dev"][j]["write"]
		if (tmp >= 0):
			log[i]["dev"][j]["write"]=tmp
f=open("binlog","w")
cPickle.dump(log,f)
f.close()
