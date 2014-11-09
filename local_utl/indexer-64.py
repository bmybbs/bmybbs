#!/usr/bin/env python
# -*- coding: utf-8 -*-

import struct, os, datetime, sys, lucene

# configure
DEBUG = 0
BBSPATH = '/home/bbs/' 
BOARDSPATH = BBSPATH+'boards/'
RECENT_INDEX = 'rindex'
UNINDEX_BOARDS = ['syssecurity','millionairesrec','newcomers',]
# end configure

def get_all_boards():
  recordformat = '24s24s9s1s454s'
  recordlen = struct.calcsize(recordformat) # the boardheader's length is 512
  
  path = BBSPATH+'.BOARDS'
  f = open(path, 'rb')
  data = f.read()
  f.close()
  filelen = os.stat(path).st_size 
  recordsnum = filelen / recordlen
  blist=[]
  for i in range(recordsnum):
      start =  i * int(recordlen)
      stop = start + int(recordlen)
      record = struct.unpack(recordformat, data[start:stop])
      bid = record[0].split('\x00')[0]
      blist.append(bid)
  return blist

def get_all_files(board, time_delta):
  recordformat = 'lllI60s14sI3Bc32s'
  recordlen = struct.calcsize(recordformat) # the fileheader's length is 144
  
  path = BOARDSPATH+board+'/.DIR'
  f = open(path, 'rb')
  data = f.read()
  f.close()
  filelen = os.stat(path).st_size 
  recordsnum = filelen / recordlen
  
  flist=[]
  for i in range(recordsnum):
    start =  i * int(recordlen)
    stop = start + int(recordlen)
    record = struct.unpack(recordformat, data[start:stop])
    filetime = long(record[0])
    title =  record[4].split('\x00')[0]
    owner =  record[5].split('\x00')[0]
	
    endday = datetime.date.today() - datetime.timedelta(days=time_delta) 
    # (time_delta == -1) denote infinite
    if (time_delta == -1 or datetime.date.fromtimestamp(filetime) >= endday):
      flist.append(('M.'+str(filetime)+'.A',owner,title))
  return flist

def filter_file(f):
  contents = ''
  
  # drop first 3 lines
  cont = 1
  for i in range(0,3):
    line = f.readline()
    if not line:
      cont = 0
      break

  while (cont):
    line = f.readline()
    if not line:
      break
    # drop attach and ref reply
    if line.startswith('beginbinaryattach') or line.startswith('【 在 '.decode('utf8').encode('gbk')):
      break
    contents += line

  return contents

def index_files(board, time_delta):
  store = lucene.SimpleFSDirectory(lucene.File(BOARDSPATH+board+'/'+RECENT_INDEX))
  writer = lucene.IndexWriter(store, lucene.StandardAnalyzer(lucene.Version.LUCENE_CURRENT), True, lucene.IndexWriter.MaxFieldLength.UNLIMITED)
#  writer.setMaxFieldLength(1048576) # 1MB
  
  flist = get_all_files(board, time_delta)
  for filename,owner,title in flist:
    path = BOARDSPATH+board+'/'+filename
    if not os.path.exists(path):
      continue

    f = open(path, 'r')
    contents = filter_file(f)
    debug(contents)
    try:
      title = title.decode('gbk')
      owner = owner.decode('gbk')
      contents = unicode(contents, 'gbk')
    except UnicodeDecodeError:
      f.close()
      debug(filename)
      continue
    f.close()
    
    if len(contents) > 0:
      doc = lucene.Document()
      doc.add(lucene.Field("name", filename,
			   lucene.Field.Store.YES,
			   lucene.Field.Index.NOT_ANALYZED))
      doc.add(lucene.Field("owner", owner,
			   lucene.Field.Store.YES,
			   lucene.Field.Index.NOT_ANALYZED))
      doc.add(lucene.Field("title", title,
			   lucene.Field.Store.YES,
			   lucene.Field.Index.NOT_ANALYZED))
      doc.add(lucene.Field("contents", contents,
			       lucene.Field.Store.YES,
			       lucene.Field.Index.ANALYZED))
      writer.addDocument(doc)
      debug('adding '+filename)
  writer.optimize()
  writer.close()

def touch_file():
  f = open(BBSPATH+'__index_end__','w')
  f.write(' ')
  f.close()

def debug(str):
  if DEBUG:
    print str

def test_fixture():
  global BOARDSPATH
  global BBSPATH
  
  BBSPATH = './parseboards/'
  BOARDSPATH = './'



if __name__ == '__main__':
  #test_fixture()

  board = sys.argv[1]
  time_delta = int(sys.argv[2])
  
  start = datetime.datetime.now()
  
  lucene.initVM()
  if board == 'ALL':
    blist = get_all_boards()
    for board in blist:
      # not in unindex list
      if UNINDEX_BOARDS.count(board) == 0:
	index_files(board, time_delta)
    # record stop time
    touch_file()
  else:
    index_files(board, time_delta) 
  end = datetime.datetime.now()
  debug(end - start)  

  
