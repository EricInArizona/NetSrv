#!/bin/bash


cd /home/Eric/NetHub

# pwd

# chmod +x aprogram.sh


# The warning about -Wpadded means it's aligning a struct
# or object on a boundary, like an 8 byte boundary.  Which
# is normal and fine.

# Can't do -Wl,--stack in Linux.
# See SetStack.cpp for that.

clang++ *.cpp /home/Eric/LinuxApi/*.cpp -o  NetHubMain -Weverything -Wno-padded -Wno-c++98-compat-pedantic -ftrapv -Ofast 2> Build.log
