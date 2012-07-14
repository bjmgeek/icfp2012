#!/usr/bin/env python
import sys

grid=[]
for line in sys.stdin:
    grid.append(line.strip('\n'))

for line in grid:
    print line
