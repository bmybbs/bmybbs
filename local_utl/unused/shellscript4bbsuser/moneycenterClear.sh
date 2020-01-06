#!/bin/bash
find /home/bbs/home/ -name "values">action.list
for i in `cat action.list`;do
  mv $i $i.bak;
  sed '/hutu/d' $i.bak>$i;
  rm $i.bak;
  mv $i $i.bak;
  sed '/St/d' $i.bak>$i;
  rm $i.bak;
  mv $i $i.bak;
  sed '/lend/d' $i.bak>$i;
  rm $i.bak;
  mv $i $i.bak;
  sed '/back/d' $i.bak>$i;
  rm $i.bak;
done
