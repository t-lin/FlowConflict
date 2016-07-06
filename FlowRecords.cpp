/* TODO: Split into header and source files later */
#include <iostream>
#include <bitset>
#include <cstdlib>
#include <unordered_map>
#include <ctime> /* Not sure how accurate this is compared to msec_timer...
                    ctime's clock() measures processor time used by program */
#include "bitscan.h" // TODO: Fix bitscan header files to avoid including multiple times
using namespace std;

// For OF 1.0, 12 fields... thus 12 bitmaps
#define NUM_OF_FIELDS 12

// How large do we need the bit arrays to be?
// Each bit in the array represents a flow in the table
// Assume max table size of 512,000 (on par with largest BGP tables today)
//#define ARRAY_SIZE 16000
//#define ARRAY_SIZE 32000
//#define ARRAY_SIZE 64000
//#define ARRAY_SIZE 128000
//#define ARRAY_SIZE 256000
#define ARRAY_SIZE 512000
//#define ARRAY_SIZE 1024000

typedef struct {
    unsigned long long dpid;
    unsigned short in_port;
    //unsigned char dl_dst[6];
    //unsigned char dl_src[6];
    unsigned long long dl_dst; // Using long long wastes 2 bytes
    unsigned long long dl_src; // Using long long wastes 2 bytes
    unsigned short dl_type;
    unsigned short dl_vlan;
    unsigned char dl_vlan_pcp;
    unsigned int nw_src;
    unsigned int nw_dst;
    unsigned char nw_proto;
    unsigned char nw_tos;
    unsigned short tp_src;
    unsigned short tp_dst;
} FlowEntry;

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

//void populateRandomBits(bitarray &array) {
//    srand(get_time_in_ms());
//    //cout << "array size is: " << array.size() << endl;
//
//    array.set_bbindex(0);
//    array.set_posbit(0);
//    for (int i = 0; i < array.size(); i++) {
//        array.set_next_bit(rand() % 2);
//    }
//}

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

/* ========== FlowRecords CLASS DEFINITION ========== */
class myGlobalClass {
    private:
        /* ===== MEMBER VARIABLES ===== */
        int mHiddenInt; // tlintest

        unordered_map<unsigned long long, bitarray> dpid_map;
        unordered_map<unsigned short, bitarray> in_port_map;
        unordered_map<unsigned long long, bitarray> dl_dst_map;
        unordered_map<unsigned long long, bitarray> dl_src_map;
        unordered_map<unsigned short, bitarray> dl_type_map;
        unordered_map<unsigned short, bitarray> dl_vlan_map;
        unordered_map<unsigned char, bitarray> dl_vlan_pcp_map;
        unordered_map<unsigned int, bitarray> nw_src_map;
        unordered_map<unsigned int, bitarray> nw_dst_map;
        unordered_map<unsigned char, bitarray> nw_proto_map;
        unordered_map<unsigned char, bitarray> nw_tos_map;
        unordered_map<unsigned short, bitarray> tp_src_map;
        unordered_map<unsigned short, bitarray> tp_dst_map;

        // Reverse lookup of flow ID -> flow entry
        unordered_map<unsigned int, FlowEntry> flowLookup;

        /* Available indices (basically IDs) to assign to a new flow
         * If bit is 1, then that index is available for use
         */
        bitarray mIndexInventory;

        /* Ensure bitarray has been initialized
         * If it has been initialized, do nothing
         * Else, initialize it
         */
        bitarray &checkArrayInit(bitarray &array);

        /* Initialize bit arrays for a flow's field values */
        //void initFlowBitArrays(const FlowEntry &flow)

        /* Returns the first available index (a 1 bit) in mIndexInventory */
        int nextFreeIndex();

        bitarray &getConflicts(const FlowEntry &flow);

        bool conflictExists(const FlowEntry &flow, const bool &printConflicts = false);

        bool atLeastOneWild(const FlowEntry &flow);

    public:
        myGlobalClass();

        ~myGlobalClass() {};

        unsigned int numFlows();

        // Returns flowID if succcessful, -1 on error
        int insertFlow(const FlowEntry &flow, bool printConflicts = false);

        // For now always returns 0...
        int removeFlow(const int &flowID);

        void increment(); // test only
};

/* ========== FlowRecords MEMBER FUNCTION DEFINITIONS ========== */
// increment() for testing only, remove later?
void myGlobalClass::increment() {
    cout << "mHiddenInt = " << ++mHiddenInt << endl;
    return;
}

myGlobalClass::myGlobalClass() {
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

inline unsigned int myGlobalClass::numFlows() {
    // The number of 0's in the index inventory
    return (ARRAY_SIZE - mIndexInventory.popcn64());
}

// Returns reference to same array object passed in
inline bitarray &myGlobalClass::checkArrayInit(bitarray &array) {
    if (array.get_bitstring() == NULL) {
        array.init(ARRAY_SIZE, true);
    }

    return array;
}

// Return index of first available bit (first 1 bit)
// Returns -1 if not found
inline int myGlobalClass::nextFreeIndex() {
    mIndexInventory.init_scan(bbo::NON_DESTRUCTIVE);
    return mIndexInventory.next_bit();
}

inline bool myGlobalClass::atLeastOneWild(const FlowEntry &flow) {
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
bitarray &myGlobalClass::getConflicts(const FlowEntry &flow) {
    static bitarray result(ARRAY_SIZE), tmp(ARRAY_SIZE), allFlows(ARRAY_SIZE);

    OR(checkArrayInit(dpid_map[flow.dpid]), dpid_map[0], result);

    /* TODO: Figure out a way to do this more elegantly
     *      If map[flow.field] doesn't exist, no need to instantiate the bitarray
     *      object or allocate a bitarray for it... this wastes time and memory
     */
    if (flow.in_port != 0)
        // Implements: result = result & ( tmp = (in_port_map[flow.in_port] | in_port_map[0]) )
        //result &= OR(in_port_map[flow.in_port], in_port_map[0], tmp);
        result &= OR(checkArrayInit(in_port_map[flow.in_port]), in_port_map[0], tmp);

    if (flow.dl_dst != 0)
        //result &= OR(dl_dst_map[flow.dl_dst], dl_dst_map[0], tmp);
        result &= OR(checkArrayInit(dl_dst_map[flow.dl_dst]), dl_dst_map[0], tmp);

    if (flow.dl_src != 0)
        //result &= OR(dl_src_map[flow.dl_src], dl_src_map[0], tmp);
        result &= OR(checkArrayInit(dl_src_map[flow.dl_src]), dl_src_map[0], tmp);

    if (flow.dl_type != 0)
        //result &= OR(dl_type_map[flow.dl_type], dl_type_map[0], tmp);
        result &= OR(checkArrayInit(dl_type_map[flow.dl_type]), dl_type_map[0], tmp);

    if (flow.dl_vlan != 0)
        //result &= OR(dl_vlan_map[flow.dl_vlan], dl_vlan_map[0], tmp);
        result &= OR(checkArrayInit(dl_vlan_map[flow.dl_vlan]), dl_vlan_map[0], tmp);

    if (flow.dl_vlan_pcp != 0)
        //result &= OR(dl_vlan_pcp_map[flow.dl_vlan_pcp], dl_vlan_pcp_map[0], tmp);
        result &= OR(checkArrayInit(dl_vlan_pcp_map[flow.dl_vlan_pcp]), dl_vlan_pcp_map[0], tmp);

    if (flow.nw_src != 0)
        //result &= OR(nw_src_map[flow.nw_src], nw_src_map[0], tmp);
        result &= OR(checkArrayInit(nw_src_map[flow.nw_src]), nw_src_map[0], tmp);

    if (flow.nw_dst != 0)
        //result &= OR(nw_dst_map[flow.nw_dst], nw_dst_map[0], tmp);
        result &= OR(checkArrayInit(nw_dst_map[flow.nw_dst]), nw_dst_map[0], tmp);

    if (flow.nw_proto != 0)
        //result &= OR(nw_proto_map[flow.nw_proto], nw_proto_map[0], tmp);
        result &= OR(checkArrayInit(nw_proto_map[flow.nw_proto]), nw_proto_map[0], tmp);

    if (flow.nw_tos != 0)
        //result &= OR(nw_tos_map[flow.nw_tos], nw_tos_map[0], tmp);
        result &= OR(checkArrayInit(nw_tos_map[flow.nw_tos]), nw_tos_map[0], tmp);

    if (flow.tp_src != 0)
        //result &= OR(tp_src_map[flow.tp_src], tp_src_map[0], tmp);
        result &= OR(checkArrayInit(tp_src_map[flow.tp_src]), tp_src_map[0], tmp);

    if (flow.tp_dst != 0)
        //result &= OR(tp_dst_map[flow.tp_dst], tp_dst_map[0], tmp);
        result &= OR(checkArrayInit(tp_dst_map[flow.tp_dst]), tp_dst_map[0], tmp);

    if (atLeastOneWild(flow)) {
        // Wildcarded field would result in match w/ all flows
        allFlows = mIndexInventory;
        result &= allFlows.flip();
    }

    return result;
}


bool myGlobalClass::conflictExists(const FlowEntry &flow, const bool &printConflicts) {
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
int myGlobalClass::insertFlow(const FlowEntry &flow, bool printConflicts) {

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
int myGlobalClass::removeFlow(const int &flowID) {
    // to do: create reverse-lookup table in insertFlow
    //  - flow-id map to list of pointers to the bitarrays??
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


