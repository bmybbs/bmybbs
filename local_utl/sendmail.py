#!/usr/bin/python
import sys, smtplib

host = 'stu.xjtu.edu.cn'
user = 'noreply'
passwd = ''

fromaddr = 'noreply@stu.xjtu.edu.cn'
toaddr = sys.argv[1]
title = sys.argv[2]
body = sys.argv[3]
msg = ("From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s\r\n"
       % (fromaddr, toaddr, title, body))


server = smtplib.SMTP(host)
#server.set_debuglevel(1)
server.login(user, passwd)
server.sendmail(fromaddr, toaddr, msg)
server.quit()
#print 'ok'
 