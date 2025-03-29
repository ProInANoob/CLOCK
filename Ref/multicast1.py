import socket
import struct



###################################################
# Common configuration items, used by both send 
# and receive:
MCAST_GRP = '224.1.1.1'
MCAST_PORT = 5007
MULTICAST_TTL = 2

# regarding socket.IP_MULTICAST_TTL (time-to-live)
# ---------------------------------
# for all packets sent, after two hops on the network the packet will not 
# be re-sent/broadcast (see https://www.tldp.org/HOWTO/Multicast-HOWTO-6.html)

#############################################################
# Create a 'transmit socket', for sending datagrams to a multicast address (group)
tx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
tx_sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, MULTICAST_TTL)



#############################################################
# Create a 'recieve socket', configured to listen to the multicast group
rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
rx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# on this port, listen ONLY to MCAST_GRP
rx_sock.bind((MCAST_GRP, MCAST_PORT))

# join the multicast group, to receive packets addressed to it
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
rx_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)


class Data(object):
    def __init__( self ):
        self.x = 0
        self.y = 0
        self.button1 = False
        self.button2 = False
    
    def toBytes( self ):
        raw = struct.pack("<ll??", self.x, self.y, self.button1, self.button2)
        return raw

    def fromBytes( self, raw ):
        values = struct.unpack( "<ll??", raw )
        self.x       = values[0]
        self.y       = values[1]
        self.button1 = values[2]
        self.button2 = values[3]

    # a convenience routine to make printout out the data easier
    def __str__( self ):
        out = "(" + str(self.x) + ", " + str(self.y) + ")"
        out += " | "
        out += "button1: " + str(self.button1) + ", "
        out += "button2: " + str(self.button2)
        return out

outgoing_data = Data()
incoming_data = Data()
        
while True:

    # change the outgoing data (as if user is moving joystick):
    outgoing_data.x += 1
    outgoing_data.y += 1
    
    # WRITE:
    # For Python 3, change next line to 'sock.sendto(b"robot", ...' to avoid the
    # "bytes-like object is required" msg (https://stackoverflow.com/a/42612820)
    tx_sock.sendto( outgoing_data.toBytes(), (MCAST_GRP, MCAST_PORT))

    # READ:
    # For Python 3, change next line to "print(sock.recv(10240))"
    inbytes = rx_sock.recv(10240)
    incoming_data.fromBytes( inbytes )
    print( incoming_data )
