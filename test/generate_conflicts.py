#!/usr/bin/python
'''
This script is used to generate conflicting flows from flows.txt
If called without any input parameters, will generate 5000 conflicting flows
Can specify number of conflicting flows generated via command line
'''

import sys

argc = len(sys.argv)
if argc == 1:
    numTargetConflicts = 5000
elif argc == 2:
    if not sys.argv[1].isdigit():
        print "Argument should be an integer"
        exit(0)

    numTargetConflicts = int(sys.argv[1])
else:
    print "Incorrect number of arguments"
    exit(0)


flowFile = open('flows.txt', 'r')
conflictFile = open('conflictingFlows.txt', 'w')

conflictCount = 0

for line in flowFile:
    flow = line.split('\t')

    # Ensure priority field is 32769
    # Check if dl_type field is filled out, if not, use this line to generate conflicts
    if flow[6] != '32769':
        continue;

    if flow[5] == '\\N':
        for i in range(1, 2**16):
            flow[5] = str(i);
            conflictLine = '\t'.join(flow)
            conflictFile.write(conflictLine)

            conflictCount += 1
            if conflictCount == numTargetConflicts:
                break;

    if conflictCount == numTargetConflicts:
        break;

conflictFile.close()
flowFile.close()

