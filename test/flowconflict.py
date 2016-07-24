#!/usr/bin/python

'''
Do as much pre-processing of JSON in Python as possible (a nightmare in C)
Then pass the relevant data to the C code to store / do conflict detection
'''

import time
from FlowRecords import FlowEntry
from FlowRecords import FlowRecords

# NOTE: For this to work... assumes that the keys used in the match dictionary
#       have the same as the FlowEntry member variables
# Format of 'flow1' taken from chaining script
#flow_name = "thomas"
#dpid = "0x10010010073"
#flow1 = {}
#flow1 = {'user_id': flow_name, 'hard_timeout': 0, 'actions': [{'type': 'OUTPUT', 'port': 2}], 'priority': 60000, 'idle_timeout': 0, 'cookie': 0}
#flow1.update({'dpid': int(dpid, 16), 'match': {'dl_type': 2048, 'nw_proto': 1, 'dl_src': "ab:cd:ef:12:34:56", 'dl_dst': "ab:cd:ef:11:22:33", 'in_port': 1}})
#
#print "Original flow JSON:\n%s\n\n" % flow1
#
## Create flow dictionary that we can work with
#userFlow = {'dpid': flow1.get('dpid')}
#userFlow.update(flow1.pop('match'))
#
#print "User flow parsed for relevant fields:\n%s\n\n" % userFlow

# Potential fields in FlowEntry class
#   - in_port
#   - dl_src
#   - dl_dst
#   - dl_type
#   - dl_vlan
#   - dl_vlan_pcp
#   - nw_src
#   - nw_dst
#   - nw_proto
#   - nw_tos
#   - tp_src
#   - tp_dst
flowRecords = FlowRecords()
flowEntry = FlowEntry()
flowEntryDict = vars(flowEntry)

#print "Flow entry before:\n%s\n\n" % vars(flowEntry)
#
#flowEntryFields = vars(flowEntry).keys() # Get member variables
#for key, val in userFlow.items():
#    if key in flowEntryFields:
#        setattr(flowEntry, key, val) # Set member variables
#
#print "Flow entry after:\n%s\n\n" % vars(flowEntry)
#
## Insert a flow
#flowID = flowRecords.insertFlow(flowEntry)
#
#if flowID >= 0:
#    print "Flow ID assigned: %s" % flowID
#else:
#    print "Flow ID %s indicates CONFLICT!" % flowID
#
## Insert another flow identical flow...
#flowID = flowRecords.insertFlow(flowEntry)
#
#if flowID >= 0:
#    print "Second Flow ID assigned: %s" % flowID
#else:
#    print "Second Flow ID %s indicates CONFLICT!" % flowID

lineCache = []
numConflicts = 0
flowCount = 0
lineCount = 0;

flowFile = open('flows.txt', 'r')
startTime = time.time()
for line in flowFile:
    lineCache.append(line)
    #lineCount += 1
    flowCount += 1
    flow = line.split('\t')

    flowEntry.reset()
    flowEntry.dpid = int(flow[1], 16)
    flowEntry.in_port = int(flow[2])
    flowEntry.dl_dst = flow[3]
    flowEntry.dl_src = flow[4] if flow[4] != '\\N' else None
    flowEntry.dl_type = int(flow[5]) if flow[5] != '\\N' else None

    if flow[6] != '32769':
        flowCount -= 1
        continue;

    if flow[9] != '\\N':
        # User ID... indicates a user flow, likely with higher priority
        # Ignore for now until we have implemented priority checking
        flowCount -= 1
        continue;

    if flow[10] != '\\N':
        # Extra fields
        flowEntryDict.update(flow[10])

    flowID = flowRecords.insertFlow(flowEntry)
    if flowID < 0:
        numConflicts += 1
        #print "FLOW #%s (line %s) HAS CONFLICT!\n%s\n" % (flowCount, lineCount, flowEntryDict)

#print "initial insertion elapsed (time): %s" % (time.time() - startTime)
flowFile.close();
#
#print "number of flows in file: %s" % flowCount
#print "number of flows stored: %s" % (flowCount - numConflicts) #len(flowRecords._RECORDS)
#print "number of conflicts: %s" % numConflicts

# Start pure conflict detection timing
#print
#print "Replaying input file for pure conflict detection"
numConflicts = 0
startTime = time.time()

#for line in flowFile:
for line in lineCache:
    #flowCount += 1
    flow = line.split('\t')

    flowEntry.reset()
    flowEntry.dpid = int(flow[1], 16)
    flowEntry.in_port = int(flow[2])
    flowEntry.dl_dst = flow[3]
    flowEntry.dl_src = flow[4] if flow[4] != '\\N' else None
    flowEntry.dl_type = int(flow[5]) if flow[5] != '\\N' else None

    if flow[6] != '32769':
        #flowCount -= 1
        continue;

    if flow[9] != '\\N':
        # User ID... indicates a user flow, likely with higher priority
        # Ignore for now until we have implemented priority checking
        #flowCount -= 1
        continue;

    if flow[10] != '\\N':
        # Extra fields
        flowEntryDict.update(flow[10])

    flowID = flowRecords.insertFlow(flowEntry)
    if flowID < 0:
        numConflicts += 1

print "conflict detection elapsed (time): %s" % (time.time() - startTime)
#print "number of conflicts: %s" % numConflicts

wait = raw_input("PRESS ENTER TO CONTINUE.")

