#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, lucene
from lucene import QueryParser, IndexSearcher, StandardAnalyzer, FSDirectory

# configure
BBSPATH = '/home/bbs/' 
BOARDSPATH = BBSPATH+'boards/'
RECENT_INDEX = 'rindex'
# end configure

def run(searcher, analyzer, querystr):
  query = QueryParser(lucene.Version.LUCENE_CURRENT, "contents", analyzer).parse(querystr)
  MAX=1000
  hits = searcher.search(query, MAX)

  results = []

  for doc in hits.scoreDocs:
    results.append([doc.get("name"), doc.get("owner").encode('gbk'), doc.get("title").encode('gbk')])
  
  # sort result
  results.sort(lambda x,y: cmp(x[0],y[0]))    
  for name,owner,title in results:
    print name, owner, title 

def test_fixture():
  global BOARDSPATH
  BOARDSPATH = './'

if __name__ == '__main__':
  #test_fixture()

  board = sys.argv[1]
  querystr = sys.argv[2].decode('gbk').strip()
  
  lucene.initVM()
  
  path = BOARDSPATH+board+'/'+RECENT_INDEX
  if not os.path.exists(path) or len(querystr) == 0:
    sys.exit(-1)
  directory = lucene.SimpleFSDirectory(lucene.File(path))
  searcher = IndexSearcher(directory)
  analyzer = StandardAnalyzer(lucene.Version.LUCENE_CURRENT)
  run(searcher, analyzer, querystr)
  searcher.close()
    
