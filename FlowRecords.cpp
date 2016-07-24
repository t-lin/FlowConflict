#include <iostream>
#include <bitset>
#include <cstdlib>
#include <unordered_map>
#include <ctime> /* Not sure how accurate this is compared to msec_timer...
                    ctime's clock() measures processor time used by program */
#include "bitscan.h"
#include "FlowRecords.h"
using namespace std;


/* ========== HELPER FUNCTIONS ========== */
unsigned long get_time_in_ms()
{
    return (unsigned long)((double(clock()) / CLOCKS_PER_SEC) * 1000);
}

void one_sec_delay()
{
    unsigned long end_time = get_time_in_ms() + 1000;

    while(get_time_in_ms() < end_time);
}

// Performs lhs &= rhs
void manualMasking(BITBOARD *lhs, BITBOARD *rhs, int numBlocks) {
    for (int i = 0; i < numBlocks; i++) {
        lhs[i] &= rhs[i];
    }

    return;
}

void printFlowEntry(const FlowEntry &flow) {
    cout << "Flow is: " << endl;
    printf("DPID: %llu, in_port: %hu\n", flow.dpid, flow.in_port);
    printf("dl_dst: %llu, dl_src: %llu, dl_type: %hu, dl_vlan: %hu, dl_vlan_pcp: %hhu\n",
            flow.dl_dst, flow.dl_src, flow.dl_type, flow.dl_vlan, flow.dl_vlan_pcp);
    printf("nw_src: %u, nw_dst: %u, nw_proto: %hhu, nw_tos: %hhu\n",
            flow.nw_src, flow.nw_dst, flow.nw_proto, flow.nw_tos);
    printf("tp_src: %hu, tp_dst: %hu\n\n", flow.tp_src, flow.tp_dst);

    return;
}

template <typename T_UMAP, typename T_KEY>
inline void AND_matchingFlows(T_UMAP &map, T_KEY &key, bitarray &result, bitarray &tmp) {
    auto iter = map.find(key);
    if (iter != map.end())
        result &= OR(iter->second, map[0], tmp);
    else
        result &= map[0];

    return;
}

/* ========== FlowRecords MEMBER FUNCTION DEFINITIONS ========== */
// increment() for testing of multi-process, remove later?
void FlowRecords::increment() {
    cout << "mHiddenInt = " << ++mHiddenInt << endl;
    return;
}

FlowRecords::FlowRecords() {
    mHiddenInt = 0;
    mIndexInventory.init(ARRAY_SIZE);
    mIndexInventory.set_bit(0, ARRAY_SIZE - 1);

    // Pre-create entries for all unordered maps? Would speed up insertion time
    // At least pre-create for wildcard fields
    dpid_map[0].init(ARRAY_SIZE, true);
    in_port_map[0].init(ARRAY_SIZE, true);
    dl_dst_map[0].init(ARRAY_SIZE, true);
    dl_src_map[0].init(ARRAY_SIZE, true);
    dl_type_map[0].init(ARRAY_SIZE, true);
    dl_vlan_map[0].init(ARRAY_SIZE, true);
    dl_vlan_pcp_map[0].init(ARRAY_SIZE, true);
    nw_src_map[0].init(ARRAY_SIZE, true);
    nw_dst_map[0].init(ARRAY_SIZE, true);
    nw_proto_map[0].init(ARRAY_SIZE, true);
    nw_tos_map[0].init(ARRAY_SIZE, true);
    tp_src_map[0].init(ARRAY_SIZE, true);
    tp_dst_map[0].init(ARRAY_SIZE, true);
}

/* Returns number of flows stored */
inline unsigned int FlowRecords::numFlows() {
    // The number of 0's in the index inventory
    return (ARRAY_SIZE - mIndexInventory.popcn64());
}

/* Ensure bitarray has been initialized
 * If it has been initialized, do nothing
 * Else, initialize it
 */
inline bitarray &FlowRecords::checkArrayInit(bitarray &array) {
    if (array.get_bitstring() == NULL) {
        array.init(ARRAY_SIZE, true);
    }

    return array;
}

/* Returns the first available index (a 1 bit) in mIndexInventory
 * Returns -1 if no available index is found
 */
inline int FlowRecords::nextFreeIndex() {
    mIndexInventory.init_scan(bbo::NON_DESTRUCTIVE);
    return mIndexInventory.next_bit();
}

/* Returns True if there is at least one field which is a wildcard
 * Otherwise, returns False
 */
inline bool FlowRecords::atLeastOneWild(const FlowEntry &flow) {
    return ( (flow.dpid == 0) || (flow.in_port == 0) || (flow.dl_dst == 0) || (flow.dl_src == 0) ||
                (flow.dl_type == 0) || (flow.dl_vlan == 0) || (flow.dl_vlan_pcp == 0) ||
                (flow.nw_src == 0) || (flow.nw_dst == 0) || (flow.nw_proto == 0) ||
                (flow.nw_tos == 0) || (flow.tp_src == 0) || (flow.tp_src == 0) );
}

/* WARNING: CURRENT IMPLEMENTATION IS NOT MULTI-THREAD SAFE
 *  - Returns reference to local static variable
 *  - TODO: Figure out a way to do this more elegantly...
 *
 * Main conflict detection algorithm here:
 *  - For each field in the input flow
 *      - Find bitarray of all existing flows where the same field can match
 *          - Can match meaning either exist match OR a wildcard value (i.e. 0)
 *  - Bit-wise AND the resulting bitarray for each field
 *  - Returns reference to bitarray where 1-bits represent the flow ID index for conflicting flows
 */
bitarray &FlowRecords::getConflicts(const FlowEntry &flow) {
    static bitarray result(ARRAY_SIZE), tmp(ARRAY_SIZE), allFlows(ARRAY_SIZE);

    //OR(checkArrayInit(dpid_map[flow.dpid]), dpid_map[0], result);
    // Having this code here explicitly initializes 'result' and avoids copying from 'tmp'
    auto dpid_iter = dpid_map.find(flow.dpid);
    if ( dpid_iter != dpid_map.end() )
        OR(dpid_iter->second, dpid_map[0], result);
    else
        result = dpid_map[0];

    /* For each field, the following operation will be performed (using in_port as an example):
     *      result = result & ( tmp = (in_port_map[flow.in_port] | in_port_map[0]) )
     */
    if (flow.in_port != 0) {
        AND_matchingFlows(in_port_map, flow.in_port, result, tmp);
        //result &= OR(checkArrayInit(in_port_map[flow.in_port]), in_port_map[0], tmp);
    }

    if (flow.dl_dst != 0) {
        AND_matchingFlows(dl_dst_map, flow.dl_dst, result, tmp);
        //result &= OR(checkArrayInit(dl_dst_map[flow.dl_dst]), dl_dst_map[0], tmp);
    }

    if (flow.dl_src != 0) {
        AND_matchingFlows(dl_src_map, flow.dl_src, result, tmp);
        //result &= OR(checkArrayInit(dl_src_map[flow.dl_src]), dl_src_map[0], tmp);
    }

    if (flow.dl_type != 0) {
        AND_matchingFlows(dl_type_map, flow.dl_type, result, tmp);
        //result &= OR(checkArrayInit(dl_type_map[flow.dl_type]), dl_type_map[0], tmp);
    }

    if (flow.dl_vlan != 0) {
        AND_matchingFlows(dl_vlan_map, flow.dl_vlan, result, tmp);
        //result &= OR(checkArrayInit(dl_vlan_map[flow.dl_vlan]), dl_vlan_map[0], tmp);
    }

    if (flow.dl_vlan_pcp != 0) {
        AND_matchingFlows(dl_vlan_pcp_map, flow.dl_vlan_pcp, result, tmp);
        //result &= OR(checkArrayInit(dl_vlan_pcp_map[flow.dl_vlan_pcp]), dl_vlan_pcp_map[0], tmp);
    }

    if (flow.nw_src != 0) {
        AND_matchingFlows(nw_src_map, flow.nw_src, result, tmp);
        //result &= OR(checkArrayInit(nw_src_map[flow.nw_src]), nw_src_map[0], tmp);
    }

    if (flow.nw_dst != 0) {
        AND_matchingFlows(nw_dst_map, flow.nw_dst, result, tmp);
        //result &= OR(checkArrayInit(nw_dst_map[flow.nw_dst]), nw_dst_map[0], tmp);
    }

    if (flow.nw_proto != 0) {
        AND_matchingFlows(nw_proto_map, flow.nw_proto, result, tmp);
        //result &= OR(checkArrayInit(nw_proto_map[flow.nw_proto]), nw_proto_map[0], tmp);
    }

    if (flow.nw_tos != 0) {
        AND_matchingFlows(nw_tos_map, flow.nw_tos, result, tmp);
        //result &= OR(checkArrayInit(nw_tos_map[flow.nw_tos]), nw_tos_map[0], tmp);
    }

    if (flow.tp_src != 0) {
        AND_matchingFlows(tp_src_map, flow.tp_src, result, tmp);
        //result &= OR(checkArrayInit(tp_src_map[flow.tp_src]), tp_src_map[0], tmp);
    }

    if (flow.tp_dst != 0) {
        AND_matchingFlows(tp_dst_map, flow.tp_dst, result, tmp);
        //result &= OR(checkArrayInit(tp_dst_map[flow.tp_dst]), tp_dst_map[0], tmp);
    }

    if (atLeastOneWild(flow)) {
        // Wildcarded field would result in match w/ all flows
        allFlows = mIndexInventory;
        result &= allFlows.flip();
    }

    return result;
}

/* Returns True if the specified flow conflicts with an existing flow entry
 * Othewise, returns False
 * Optional parameter: printConflicts for printing any conflicting flows identified
 */
bool FlowRecords::conflictExists(const FlowEntry &flow, const bool &printConflicts) {
    bitarray result = getConflicts(flow);
    bool bConflict = result.popcn64();

    if (bConflict && printConflicts) {
        int index = 0;
        result.init_scan(bbo::NON_DESTRUCTIVE);
        while ( (index = result.next_bit()) >= 0 ) {
            printFlowEntry(flowLookup[index]);
        }
    }

    return bConflict;
}

// Returns assigned flow ID on success
// Returns -1 on failure
int FlowRecords::insertFlow(const FlowEntry &flow, bool printConflicts) {

    // Do conflict detection here
    if (conflictExists(flow, printConflicts))
        return -1;

    register int flowID = nextFreeIndex(); // Going to be read a lot very quickly (OF13 has 44 fields...)

    if (flowID != -1) {
        checkArrayInit(dpid_map[flow.dpid]).set_bit(flowID);
        checkArrayInit(in_port_map[flow.in_port]).set_bit(flowID);
        checkArrayInit(dl_dst_map[flow.dl_dst]).set_bit(flowID);
        checkArrayInit(dl_src_map[flow.dl_src]).set_bit(flowID);
        checkArrayInit(dl_type_map[flow.dl_type]).set_bit(flowID);
        checkArrayInit(dl_vlan_map[flow.dl_vlan]).set_bit(flowID);
        checkArrayInit(dl_vlan_pcp_map[flow.dl_vlan_pcp]).set_bit(flowID);
        checkArrayInit(nw_src_map[flow.nw_src]).set_bit(flowID);
        checkArrayInit(nw_dst_map[flow.nw_dst]).set_bit(flowID);
        checkArrayInit(nw_proto_map[flow.nw_proto]).set_bit(flowID);
        checkArrayInit(nw_tos_map[flow.nw_tos]).set_bit(flowID);
        checkArrayInit(tp_src_map[flow.tp_src]).set_bit(flowID);
        checkArrayInit(tp_dst_map[flow.tp_dst]).set_bit(flowID);

        // Save flowEntry for reverse lookup (invoke the move constructor)
        // Assumes the FlowEntry flow object was not dynamically allocated...
        flowLookup[flowID] = move(flow);

        // Un-set flowID bit in mIndexInventory to mark that index as reserved
        mIndexInventory.erase_bit(flowID);
    }

    return flowID;
}

// This should really be a void... since set and erase bit functions are voids
int FlowRecords::removeFlow(const int &flowID) {
    if (mIndexInventory.is_bit(flowID)) {
        FlowEntry flow = move(flowLookup[flowID]);

        /* In theory, shouldn't need checkArrayInit here... extra unnecessary overhead
         * Call to insertFlow should have ensured the bitarrays have been initialized
         * Keeping it for now "just in case" some odd call sequence gets here without initialization...
         * TODO: Benchmark overhead, if it impacts performance, remove the check
         */
        checkArrayInit(dpid_map[flow.dpid]).erase_bit(flowID);
        checkArrayInit(in_port_map[flow.in_port]).erase_bit(flowID);
        checkArrayInit(dl_dst_map[flow.dl_dst]).erase_bit(flowID);
        checkArrayInit(dl_src_map[flow.dl_src]).erase_bit(flowID);
        checkArrayInit(dl_type_map[flow.dl_type]).erase_bit(flowID);
        checkArrayInit(dl_vlan_map[flow.dl_vlan]).erase_bit(flowID);
        checkArrayInit(dl_vlan_pcp_map[flow.dl_vlan_pcp]).erase_bit(flowID);
        checkArrayInit(nw_src_map[flow.nw_src]).erase_bit(flowID);
        checkArrayInit(nw_dst_map[flow.nw_dst]).erase_bit(flowID);
        checkArrayInit(nw_proto_map[flow.nw_proto]).erase_bit(flowID);
        checkArrayInit(nw_tos_map[flow.nw_tos]).erase_bit(flowID);
        checkArrayInit(tp_src_map[flow.tp_src]).erase_bit(flowID);
        checkArrayInit(tp_dst_map[flow.tp_dst]).erase_bit(flowID);

        // Set this index (ID) to be available for re-use,
        // and also erase the flow entry from flowLookup
        mIndexInventory.set_bit(flowID);
        flowLookup.erase(flowID);
    }

    return 0;
}


