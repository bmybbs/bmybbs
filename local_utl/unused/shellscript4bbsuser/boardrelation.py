#!/usr/bin/python
# -*- coding: utf-8 -*-

# 通过bbslogd(newtrace)日志,生成版面相关度
# author: interma@bmy

import struct, os, datetime, sys

BBS_DIR = '/home/bbs/'
LOG_DIR = BBS_DIR+'newtrace/'
BOARD_DIR = BBS_DIR+'boards/'

log_list = [] # 日志列表

ba = {} # 版面:所在区 字典
pb = {} # id:版面 评分字典
pbf = {} # id:版面 评分字典(已排序)
bp = {} # 版面:id 评分字典
bpf = {} # 版面:id 评分字典(已排序)
bb = {} # 版面:版面 评分字典
bbf = {} # 版面:版面 评分字典(已排序)

# 版面相关度评分
def board_score(b1, b2):
    global ba
    if not ba.has_key(b1) or not ba.has_key(b2):
        return 0
    ret = 0
    a1 = ba[b1]
    a2 = ba[b2]
    if a1 == a2:
        ret = 8
    elif a2=='2' or a2=='3' or a2=='4' or a2=='5' or a2=='6' or a2=='7' or a2 =='8' or a2 == '9' or a2 =='G' :
        ret = 4
    elif a2 =='1' or a2 == 'N' or a2 == 'H' or a2 == 'C':
        ret = 2
    else:
        ret = 1
    return ret

# 对元组的升序比较函数
def my_cmp(E1, E2):
    return -cmp(E1[1], E2[1])

# 生成前7天的loglist(不包含今天)
def gen_loglist():
    global log_list
    today = datetime.date.today( )
    for i in range(1,8):
        tmp = today - datetime.timedelta(days=i)
        log_list.append(str(tmp)+'.log')

# 生成ba
def gen_ba():
    global ba
    filepath = BBS_DIR+'.BOARDS'
    recordformat = '24s24s9s1s454s'
    recordlen = struct.calcsize(recordformat) # the boardheader's length is 512

    f = open(filepath, 'rb')
    data = f.read()
    f.close()
    filelen = os.stat(filepath).st_size 
    recordsnum = filelen / recordlen
    for i in range(recordsnum):
        start =  i * int(recordlen)
        stop = start + int(recordlen)
        record = struct.unpack(recordformat, data[start:stop])
        #ba[record[0].strip('\x00')] = record[3]
        bid = record[0].split('\x00')[0]
        ba[bid] = record[3]
        

# 生成pb,bp
def gen_pb_bp():
    global pb,bp,log_list
    for filepath in log_list:
        filepath = LOG_DIR+filepath
        if not os.path.exists(filepath):
            continue
    	f = open(filepath, 'r')
    	for line in  f.readlines():
    		segs = line.split()
    		if (len(segs) < 4):
    			continue
    		person = segs[1]
    		action = segs[2]
    		board = segs[3]
    		usetime = 0
            # 不考虑guest的访问
    		if person == 'guest':
    			continue
    		if action == 'use':
    			usetime = int(segs[-1])
    		elif action == 'post':
    			usetime = 60
    		else:
    			continue
            # 防止版面挂id
    		if usetime > 60:
    			usetime = 60
    			
    		if not pb.has_key(person):
    			pb[person] = {}
    		if not pb[person].has_key(board):
    			pb[person][board] = 0;
    		if not bp.has_key(board):
    			bp[board] = {}
    		if not bp[board].has_key(person):
    			bp[board][person] = 0;
    		
    		pb[person][board] = pb[person][board] + usetime
    		bp[board][person] = bp[board][person] + usetime
    	f.close()  
 
# 生成pbf
def gen_pbf():  
    global pb,pbf
    for k,v in pb.items():
        pbf[k] = []
        for kk,vv in v.items():
            pbf[k].append((kk, vv))
        pbf[k].sort(my_cmp)
      
# 生成bpf
def gen_bpf():
    global bp,bpf
    for k,v in bp.items():
        bpf[k] = []
        for kk,vv in v.items():
            bpf[k].append((kk, vv))
        bpf[k].sort(my_cmp)

# 生成bb    
def gen_bb():
    global bb,pbf
    for k,v in bpf.items():
        bb[k] = {}
        for person in v[0:20]: # 默认每版前20个id
            person = person[0]
            for board in pbf[person][0:5]: # 默认每个id前5个版面
                board = board[0]
                if not bb[k].has_key(board):
                    bb[k][board] = 0
                bb[k][board] = bb[k][board] + board_score(k,board)
        bb[k][k] = 0
    
# 生成bbf        
def gen_bbf():
    global bb,bbf
    for k,v in bb.items():
        bbf[k] = []
        for kk,vv in v.items():
            bbf[k].append((kk, vv))
        bbf[k].sort(my_cmp)

# 在各个版面的主目录下生成文件
def gen_file():        
    global bbf
    for k in bbf.keys():
        filepath = BOARD_DIR+k+'/'
        if not os.path.exists(filepath):
            continue        
        f = open(filepath+'boardrelation', 'w')
        end = 5 # 默认取前5个版面
        if len(bbf[k]) < 5:
            end = len(bbf[k])
        for i in range(0,end):
            if bbf[k][i][1] != 0:
                f.write(bbf[k][i][0]+',')
        f.close()
    

if __name__ == '__main__':
    gen_loglist()
    gen_ba()
    gen_pb_bp()
    if len(pb) == 0:
        sys.exit(1)
    gen_pbf()
    gen_bpf()
    gen_bb()
    gen_bbf()
    gen_file()
    
