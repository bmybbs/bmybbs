gzip -d -c %1 >temp.tar
tar -x -m --atime-preserve -f temp.tar
del temp.tar