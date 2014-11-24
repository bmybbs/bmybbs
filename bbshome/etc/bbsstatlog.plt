set terminal png small color
set size 1,0.75
set output "/home/bbs/bbstmpfs/dynamic/online.png"
set timestamp
set xrange [0:24]
set xtics 0,2,24
set key top left Left reverse
set grid
set lmargin 8
plot "/home/bbs/reclog/bbsstatproc.log" u 1:2 title "recent 24 hours" with l 1,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:3 title "average" with l 2,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:4 title "telnet" with l 3,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:5 title "average" with l 4,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:6 title "www" with l 8,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:7 title "average" with l 5,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:12 title "wwwguest" with l 7,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:13 title "average" with l 0
set size 1,0.4
set output "/home/bbs/bbstmpfs/dynamic/netflow.png"
plot "/home/bbs/reclog/bbsstatproc.log" u 1:($14/1024.) title "netflow (Mbit/s)" with l 1,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:($15/1024.) title "average" with l 2
set terminal png small color
set size 1,0.6
set output "/home/bbs/bbstmpfs/dynamic/load.png"
set key
set logscale y
set yrange [0.01:10]
plot "/home/bbs/reclog/bbsstatproc.log" u 1:($8>0?$8:0.005) title "recent 24 hours" with l 1,\
     "/home/bbs/reclog/bbsstatproc.log" u 1:($9>0?$9:0.005) title "average" with l 8

