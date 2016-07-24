//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <cstdlib>
#include <unordered_map>
#include <ctime> /* Not sure how accurate this is compared to msec_timer...
                    ctime's clock() measures processor time used by program */
#include "bitscan.h"

#include "FlowRecords.h"
using namespace std;

// How large do we need the bit arrays to be?
// Each bit in the array represents a flow in the table
// Assume max table size of 512,000 (on par with largest BGP tables today)
//#define ARRAY_SIZE 1024000
#define ARRAY_SIZE 512000

/*
 * 1st step: duplicate flowvisor logic? https://github.com/t-lin/flowvisor/blob/1.4-MAINT/src/org/flowvisor/flows/FlowSpaceRuleStore.java
 *  - Need to create FlowEntry class
 *      - Each Flow hsa a unique ID (turn it into a string, and hash it?)
 *          - Would be easy to see if it already exists using Bloom Filter
 *      - Need to create series of key->BitSet maps
 *      - Need special keys for ANY-* of each field
 *  - What to use as index of bitarray? Index should be uniquely matched to a flow...
 *      - Just assign index as new flows come in?
 *          - TO DO: Create map/hash between a flow (in dict format) and an integer representing bit index position
 *      - FlowVisor's method: Use their ID from an sql db... but that is monotonically increasing
 *          - Occasionally calls a defrag function to re-set all their IDs
 *  - For each field in OF, it has a map from from key (the field value) to bitmap
 *      - This bitmap represents all the flows who has a matching key
 */

/* Bitset options:
 *  - std::bitset
 *      - Stores entire bitset on stack
 *  - vector<bool>
 *      - Specially customized to optimize for space
 *  - BITSCAN
 *      - Provides class functions for doing optimized scanning
 *      - Also has class functions for bit-wise operations
 */



int main()
{
    FlowRecords test;

    test.increment();

    //cout << "test.mHiddenInt: " << test.mHiddenInt << endl;
    int flowID = 0;
    FlowEntry origFlow = {0}, flow = {0};

    origFlow.dpid = 1099780128883;
    origFlow.in_port = 1;
    origFlow.dl_dst = 188900967522867;
    origFlow.dl_src = 188900967593046;
    origFlow.dl_type = 2048;
    //origFlow.dl_vlan = 0;
    //origFlow.dl_vlan_pcp = 0;
    //origFlow.nw_src = 0;
    //origFlow.nw_dst = 0;
    //origFlow.nw_proto = 0;
    //origFlow.nw_tos = 0;
    //origFlow.tp_src = 0;
    //origFlow.tp_dst = 0;

    /* ========== TEST 1: Basic insert ========== */
    flow = origFlow;
    flowID = test.insertFlow(flow);
    cout << "First flow insertion attempt. Got flow ID: " << flowID << endl;

    /* ========== TEST 2: Insert another identical flow ========== */
    flowID = test.insertFlow(flow);
    cout << "Second flow insertion attempt. Got flow ID: " << flowID << endl;

    /* ========== TEST 3: Insert "superset" flow  ========== */
    flow.dl_dst = 0; // wildcard
    flow.dl_src = 0;
    flowID = test.insertFlow(flow);
    cout << "Third flow insertion attempt. Got flow ID: " << flowID << endl;

    /* ========== TEST 4: Insert "intersect" flow ========== */
    flow = origFlow;
    flow.dl_dst = 0;
    flow.dl_src = 0;
    flow.dl_vlan = 100;
    flow.nw_proto = 6;
    flowID = test.insertFlow(flow);
    cout << "Fourth flow insertion attempt. Got flow ID: " << flowID << endl;

    /* ========== TEST 4: Insert "subset" flow ========== */
    flow = origFlow;
    flow.dl_vlan = 100;
    flow.nw_proto = 6;
    flowID = test.insertFlow(flow, true);
    cout << "Fourth flow insertion attempt. Got flow ID: " << flowID << endl;

    /* ========== TEST 5: Insert non-conflict different flow ========== */
    flow = {0};
    flow.dpid = 1099780128883;
    flow.in_port = 1;
    flow.dl_dst = 288900967522867; // diff dest mac
    flow.dl_src = 288900967593046; // diff src mac
    flow.dl_type = 2048;
    flowID = test.insertFlow(flow);
    cout << "Fifth flow insertion attempt. Got flow ID: " << flowID << endl;


    return 0;
}
