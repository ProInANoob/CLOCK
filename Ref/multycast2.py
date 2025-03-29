
import socket
import struct
import time
import pygame

###################################################
# Common configuration items, used by both send 
# and receive:
MCAST_GRP = '224.1.1.1'
MCAST_PORT_CONTROLER = 5007
MCAST_PORT_SENSOR = 5006
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
rx_sock.bind(("0.0.0.0", MCAST_PORT_SENSOR))

# join the multicast group, to receive packets addressed to it
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
rx_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)


class PS4(object):
    """Class representing the PS4 controller. Pretty straightforward functionality."""
    controller = None
    axis_data = None
    button_data = None
    hat_data = None
    
    def init(self):
        """Initialize the joystick components"""
        pygame.init()
        pygame.joystick.init()
        self.controller = pygame.joystick.Joystick(0)
        self.controller.init()
        
        
    def listen(self):
        """Listen for events to happen"""       
        if not self.axis_data:
            self.axis_data = {}
            self.axis_data[1] = 0
            self.axis_data[3] = 0
            
            
        if not self.button_data:
            self.button_data = {}
            for i in range(self.controller.get_numbuttons()):
                self.button_data[i] = False
                
        if not self.hat_data:
            self.hat_data = {}
            for i in range(self.controller.get_numhats()):
                self.hat_data[i] = (0, 0)
        
        for event in pygame.event.get():
            if event.type == pygame.JOYAXISMOTION:
                self.axis_data[event.axis] = int(event.value*100)
            elif event.type == pygame.JOYBUTTONDOWN:
                self.button_data[event.button] = True
            elif event.type == pygame.JOYBUTTONUP:
                self.button_data[event.button] = False
            elif event.type == pygame.JOYHATMOTION:
                self.hat_data[event.hat] = event.value
                # Insert your code on what you would like to happen for each event here!
                # In the current setup, I have the state simply printing out to the screen.
                


class Controler_Data(object):
    def __init__( self ):
        self.x = 0
        self.y = 0
        self.up = False
        self.down = False
        self.left = False
        self.right = False
    
    def toBytes( self ):
        # change this to receve sensor data
        raw = struct.pack("<ll????", self.x, self.y, self.up, self.down, self.left, self.right)
        return raw

    def fromBytes( self, raw ):
        values = struct.unpack( "<ll????", raw )
        self.x       = values[0]
        self.y       = values[1]
        self.up      = values[2]
        self.down    = values[3]
        self.left    = values[4]
        self.right   = values[5]

    # a convenience routine to make printout out the data easier
    def __str__( self ):
        out = "(" + str(self.x) + ", " + str(self.y) + ")"
        out += " | "
        out += "up: " + str(self.up) + ", "
        out += "down: " + str(self.down) + ", "
        out += "left: " + str(self.left) + ", "
        out += "right: " + str(self.right)
        return out
    
class Sensor_Data(object):
    def __init__( self ):
        self.USDis = 0
        self.temp = 0
        self.humid = 0
        self.co2 = 0
        self.tvoc = 0
        self.hall = 0
        
    
    def toBytes( self ):
        # change this to receve sensor data
        raw = struct.pack("<lllllll", self.USDis, self.temp, self.humid, self.co2, self.tvoc, self.hall)
        return raw

    def fromBytes( self, raw ):
        values = struct.unpack( "<llllll", raw )
        self.USDis = values[0]
        self.temp = values[1]
        self.humid = values[2]
        self.co2 = values[3]
        self.tvoc = values[4]
        self.hall = values[5]
        

    # a convenience routine to make printout out the data easier
    def __str__( self ):
        out = "dist = " + str(self.USDis) + ",  temp " + str(self.temp)
        out += " humid: " + str(self.humid) + ", "
        out += "co2: " + str(self.co2) + ", "
        out += "tvoc: " + str(self.tvoc) + ", "
        out += "hall: " + str(self.hall)
        return out

outgoing_data = Controler_Data()
incoming_data = Sensor_Data()
ps4 = PS4()
ps4.init()
while True:
    
  
    ps4.listen()
    # change the outgoing data (as if user is moving joystick):
    outgoing_data.x = ps4.axis_data[1]
    outgoing_data.y = ps4.axis_data[3]
    outgoing_data.up = ps4.button_data[3]
    outgoing_data.down = ps4.button_data[0]
    outgoing_data.left = ps4.button_data[2]
    outgoing_data.right = ps4.button_data[1]
    # WRITE:
    # For Python 3, change next line to 'sock.sendto(b"robot", ...' to avoid the
    # "bytes-like object is required" msg (https://stackoverflow.com/a/42612820)
    tx_sock.sendto( outgoing_data.toBytes(), (MCAST_GRP, MCAST_PORT_CONTROLER))
    print(outgoing_data.__str__())
    # READ:
    # For Python 3, change next line to "print(sock.recv(10240))"
    inbytes = rx_sock.recv(10240)
    incoming_data.fromBytes( inbytes )
    #print( incoming_data)
    time.sleep(.1)
