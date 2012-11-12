#!/usr/bin/env python
# auto delete big file
# author: InterMa@BMY 2006-6-30

import os, sys
from time import *
from stat import *

# begin configure
file_list = ['php-4.3.11.tar.gz', 'ruby-1.8.4.tar.gz'] # the file which want to examine
MAX_FILE_SIZE = 1024 * 1024 * 2 # max file size, unit: byte 
# end configure 

# program begin
for file in file_list:
    if os.path.exists(file):
        status = os.stat(file);
        mode = status.st_mode
        if S_ISREG(mode):
            if status.st_size > MAX_FILE_SIZE:
                os.unlink(file)
                print '[Done] already delete ', file, ' ', ctime(time())
        else:
            print '[Error]', file, ' is not a file!'
    else:
        print '[Error]', file, ' is not exist!'
