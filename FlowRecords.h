#ifndef FLOWRECORDS_H
#define FLOWRECORDS_H

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

/* Define FlowEntry struct/object
 * Currently for OpenFlow 1.0
 * NOTE: Currently 4 bytes larger than needed, this is largely for ease when
 *       doing copy or comparison operations (no need for byte-by-byte)
 */
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


/* ========== FlowRecords CLASS DEFINITION ========== */
class FlowRecords {
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

        /* Returns the first available index (a 1 bit) in mIndexInventory
         * Returns -1 if no available index is found
         */
        int nextFreeIndex();

        /* Main conflict detection algorithm here */
        bitarray &getConflicts(const FlowEntry &flow);

        /* Returns True if the specified flow conflicts with an existing flow entry
         * Othewise, returns False
         * Optional parameter: printConflicts for printing any conflicting flows identified
         */
        bool conflictExists(const FlowEntry &flow, const bool &printConflicts = false);

        /* Returns True if there is at least one field which is a wildcard
         * Otherwise, returns False
         */
        bool atLeastOneWild(const FlowEntry &flow);

    public:
        FlowRecords();

        ~FlowRecords() {};

        /* Returns number of flows stored */
        unsigned int numFlows();

        // Returns flowID if succcessful, -1 on error
        int insertFlow(const FlowEntry &flow, bool printConflicts = false);

        // For now always returns 0...
        int removeFlow(const int &flowID);

        // increment() for testing of multi-process, remove later?
        void increment();
};


#endif
