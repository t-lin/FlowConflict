# vim: tabstop=4 shiftwidth=4 softtabstop=4
#
# Copyright (C) 2013, The SAVI Project.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Import tlintest here will create a global instance of the flow store
import tlintest

# FlowRecords class to call the actual C extensions
# Can think of this class as an intermediate layer whose job is to ensure
# that all function calls to the C code has been properly type-checked
# and vetted to avoid any errors from C (which are harder to debug)
class FlowRecords(object):
    def __init__(self):
        # Default values are also "don't care"/wildcard value
        # No default datapath ID value because that is always required

        # Map from flow ID to flow entry dictionary in numerical form
        self._RECORDS = {}

    # Translates from Python values to C values
    def _setDefaults(self, flowDict):
        assert type(flowDict) is dict

        for k,v in flowDict.items():
            if v is None:
                flowDict[k] = 0

    def _mac2int(self, mac):
        assert type(mac) is str
        macInt = 0;
        byteList = mac.split(':')
        assert len(byteList) == 6

        for byte in byteList:
            macInt = (macInt << 8) | int(byte, 16)

        return macInt

    #def _int2mac(self, integer):
    #    assert type(integer) is int
    #    mac = ""
    #    while integer:
    #        byte = integer & 0xFF
    #        mac = ("%02x" % byte) + mac
    #        integer >> 8

    #    return mac

    def _ip2int(self, ip):
        assert type(ip) is str
        ipInt = 0;
        byteList = ip.split('.')
        assert len(byteList) == 4

        for byte in byteList:
            ipInt = (ipInt << 8) | int(byte)

        return ipInt

    # First, translates MACs and IPs to numbers (easier comparison in C)
    def insertFlow(self, flow):
        assert type(flow) is FlowEntry

        # Create copy of flow and modify its entries
        flowEntry = FlowEntry()
        flowDict = vars(flowEntry)
        flowDict.update(vars(flow))

        if flowEntry.dl_dst:
            flowEntry.dl_dst = self._mac2int(flowEntry.dl_dst)

        if flowEntry.dl_src:
            flowEntry.dl_src = self._mac2int(flowEntry.dl_src)

        if flowEntry.nw_src:
            flowEntry.nw_src = self._ip2int(flowEntry.nw_src)

        if flowEntry.nw_dst:
            flowEntry.nw_dst = self._ip2int(flowEntry.nw_dst)

        # Call setDefaults to parse out 'None's and replace with 0
        self._setDefaults(flowDict)

        flowID = tlintest.insertFlow(**flowDict)
        if flowID >= 0:
            #self._RECORDS[flowID] = dict(flowDict) # Deep copy
            pass

        return flowID


# FlowEntry class for easy Python manipulation of flow entries
class FlowEntry(object):
    def __init__(self):
        # Initialize OF flow match's 12-tuple to all None (wildcards)
        self.in_port = None
        self.dl_src = None
        self.dl_dst = None
        self.dl_type = None
        self.dl_vlan = None
        self.dl_vlan_pcp = None
        self.nw_src = None
        self.nw_dst = None
        self.nw_proto = None
        self.nw_tos = None
        self.tp_src = None
        self.tp_dst = None

        # DPID
        self.dpid = None

        # Not part of OF's 12-tuple; Used for deleting flows only
        #self.out_port = None

        # Initialize list of actions to empty list
        # Each action in the list is of a FlowAction class object
        #self.actions = []

    #def addAction(self, action):
    #    assert isinstance(action, FlowAction)
    #    self.actions.append(action)

    #def getActions(self):
    #    return self.actions

    def isAllWild(self):
        allWild = not (self.in_port or self.dl_src or self.dl_dst or \
                        self.dl_type or self.dl_vlan or self.dl_vlan_pcp or \
                        self.nw_src or self.nw_dst or self.nw_proto or \
                        self.nw_tos or self.tp_src or self.tp_dst)
        return allWild

    def reset(self):
        self.__init__()

    def increment(self):
        tlintest.increment()


    """
    def set_in_port(self, port):
        self.in_port = port

    def set_dl_src(self, src):
        self.dl_src = src

    def set_dl_dst(self, dst):
        self.dl_dst = dst

    def set_dl_type(self, type):
        self.dl_type = type

    def set_dl_vlan(self, vlan):
        self.dl_vlan = vlan

    def set_dl_vlan_pcp(self, vlan_pcp):
        self.dl_vlan_pcp = vlan_pcp

    def set_nw_src(self, src):
        self.nw_src = src

    def set_nw_dst(self, dst):
        self.nw_dst = dst

    def set_nw_proto(self, proto):
        self.nw_proto = proto

    def set_nw_tos(self, tos):
        self.nw_tos = tos

    def set_tp_src(self, src):
        self.tp_src = src

    def set_tp_dst(self, dst):
        self.tp_dst = dst

    def get_flow_dict(self):
        pass
    """


