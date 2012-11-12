#!/usr/bin/python
import sys,urllib,re
if len(sys.argv) < 3:
	print "Faint! no enough argument!"
	sys.exit(-1)
userid=sys.argv[1]
password=sys.argv[2]

f = urllib.urlopen("http://162.105.138.200/uhtbin/cgisirsi/0/0/29/50/X/1")
data=f.readlines()
f.close()
pos=0
for i in range(len(data)):
	if data[i]=='<FORM NAME="accessform" METHOD="POST"\n':
		pos=i
		break
if(pos==0):
	print "no accessform"
	sys.exit(-1)
posturl=data[pos+1]
m=re.match("\s+ACTION=\"(?P<url>/uhtbin/cgisirsi/[a-zA-Z0-9/]*)\">",posturl)
if(m==None):
	print "no match!"
	sys.exit(-1)
url=m.group("url")
params = urllib.urlencode({'user_id': userid, 'password': password})
f = urllib.urlopen("http://162.105.138.200"+url,params)
data=f.read()
f.close()
print data
