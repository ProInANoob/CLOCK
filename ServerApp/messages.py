import struct

MCAST_GRP   = '224.1.1.1' # in range ()



MCAST_PORT        = 5006 # laptop one. 
MCAST_PORT_CLOCK  = 5007 # wait I can use addresses I thinkkkkk look at https://docs.python.org/3/library/socket.html#socket-objects
MCAST_PORT_BLUE   = 5008
MCAST_PORT_ORANGE = 5009

class to_clock_data():
    def __init__(self, tx):
        self.tx_sock = tx
        self.startClock = 0
        self.reset = 0
        self.pause = 0
        self.win = 0
        self.readyBlue = 0
        self.readyOrange = 0
        self.orangeTapin = 0
        self.blueTapin = 0

    def toBytes( self ):
        raw = struct.pack("<???l????", self.startClock, self.reset, self.pause, self.win, self.readyBlue, self.readyOrange, self.orangeTapin, self.blueTapin)
        return raw
    
    def send(self):
        raw = self.toBytes()
        self.tx_sock.sendto( raw, (MCAST_GRP, MCAST_PORT_CLOCK))

class from_clock_data():
    def __init__(self):
        self.running = 0
        self.reset_ack = 0
        self.done = 0

    def fromBytes(self, raw):
        vals = struct.unpack("<???", raw)
        self.running   = vals[0]
        self.reset_ack = vals[1]
        self.done      = vals[2]
    
    #might write a send funck here for the to witch button stuff, actualy I should do that with aall the sends. 

class to_button_data():

    

    def __init__( self, tx ):
        self.tx_sock = tx
        self.color = -1
        self.main_ack = 0
        self.tap_ack = 0

    def  toBytes( self ):
        raw = struct.pack("<l??", self.color, self.main_ack, self.tap_ack) # wait how endian 0 = ?, 1= =? l = 2:6 ithink.
        return raw
    
    def assignCol(self, col):
        self.col = col


    def send( self ):
        raw = self.toBytes()
        #raw = (raw << 2) + 0x3
        if self.color == 1:
           self.tx_sock.sendto( raw, (MCAST_GRP, MCAST_PORT_BLUE))
        else:
            self.tx_sock.sendto( raw, (MCAST_GRP, MCAST_PORT_ORANGE))

class from_button_data():
    def __init__(self):
        self.mainPress = 0
        self.tapoutPress = 0

    def fromBytes(self, raw):
        vals = struct.unpack("<??", raw)
        self.mainPress   = vals[0]
        self.tapoutPress = vals[1]