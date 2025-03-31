
import socket
import struct
import time
import pygame
import messages
import gui
import time
import sys
import errno

Gui = gui.GUI()


# I 'can' only send on one port, so i need to differectiate between goes to clock and  goes to button.   I thing data << 1 + 1 or +0, then check %2, on the arduino end to see.

# okay state def itme
# TO CLOCK:
# bool start clock - toggle on an ack from cclock
# bool reset - another ack toggle.
# bool pause - this  will just stay for len of pause.
# int win - 0 = none (when clock is done, should go to yellow, log on clock side), 2 = B, 1 = orange. 
# bool readyBlue - fro gears, just bool passed form button (sets to green I think)
# bool readyORANGE - - fro gears, just bool passed form button (sets to green I think)
# bool orangeTapin - also for gears (set to color), enables button, and idk 
# bool Blue tapin - also for gears (set to color), enables button, and idk 
#FROM CLOCK:
# bool running - start clock ack & state
# bool reset - ack
# bool done (clock at 0)
#
#TO BUTTONS: ( for this for independance, ol do ther are <<2, and 2nd bit is witch button, )
# int color - setup key in a sec ( this can be button sepecific now.)
# MainAck - because of udp and nonblocking ect...  
# tapAck - because of udp and nonblocking ect... 
#FROM BUTTON
# bool: mainPress
# bool: tapoutPress
###################################################
# Common configuration items, used by both send 
# and receive:
MCAST_GRP   = "224.1.1.1" # in range ()



MCAST_PORT        = 5006 # laptop one. 
MCAST_PORT_CLOCK  = 5007 # wait I can use addresses I thinkkkkk look at https://docs.python.org/3/library/socket.html#socket-objects
MCAST_PORT_BLUE   = 5008
MCAST_PORT_ORANGE = 5009

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
# Create a 'recieve socket', configured to listen to the multicast group (clock port)
rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
rx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# on this port, listen ONLY to MCAST_GRP
rx_sock.bind(("0.0.0.0", 5006))
rx_sock.setblocking(False)

# join the multicast group, to receive packets addressed to it
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
rx_sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)





# color key: -1 = off, 0 = standby color, 1 = ready color(orange, blue), 2 = win color, 3 = loose color (Red, probably)


def parseData( data ): # im just gonna add an int to all of these
    print(data)
    #if(data[0] == 0x1):
    #    print("fromClock")
    #    fromClock.fromBytes(data[4:])
    #    return 1
    #elif(data[0] == 0x2):
    #    print("fromOrange")
    #    fromOrange.fromBytes(data[4:])
    #    return 1
    #elif(data[0] == 0x3):
    #    print("FromBlue")
    #    fromBlue.fromBytes(data[4:])
    #    return 1
    #else:
    #    return 0
    
toClock    = messages.to_clock_data(tx_sock)
toOrange   = messages.to_button_data(tx_sock)
toOrange.assignCol(1)
toBlue     = messages.to_button_data(tx_sock)
toBlue.assignCol(0)
fromOrange = messages.from_button_data()
fromBlue   = messages.from_button_data()
fromClock  = messages.from_clock_data()

fastTimer =  time.time()


orangeTapin = False
blueTapin = False
clcokDone = False

while not gui.glfw.window_should_close(Gui.window):
    
    Gui.loop(toBlue, fromBlue, toOrange, fromOrange, toClock, fromClock)
    orangeTapin = toClock.orangeTapin
    blueTapin = toClock.blueTapin

    
    if (time.time() - fastTimer) > 0.1: # in sec
        # WRITE:
        fastTimer = time.time()
        #print("sending things")
        toBlue.send()
        toClock.send()

        # For Python 3, change next line to 'sock.sendto(b"robot", ...' to avoid the
        # "bytes-like object is required" msg (https://stackoverflow.com/a/42612820)
        
        # READ:
        # For Python 3, change next line to "print(sock.recv(10240))"
        try:
            indata = rx_sock.recv(1024)
            #print("after")
            
        except socket.error as e:
            err = e.args[0]
            e.args[0]
            if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
                #print ('No data available') 
                continue 
            else: # a "real" error occurred print
                sys.exit(1) 
        else: # got a message
            print("DATATATAT")    
            pass   

        #print(len(indata))
        if(len(indata) > 1):
            parseData( indata )
            #print("parsisisnd")

        # Check changes and set acks?
        

        # TOCLCOK
        #3start int gui
        #3reset in gui
        #3pause in gui
        #win can be tapout ( 1= orange win)
        if(fromBlue.tapoutPress ==1 or fromOrange.tapoutPress == 1):
            toClock.win = 1 if fromBlue.tapoutPress == 1 else 2
            if toClock.win == 1:
                toBlue.tap_ack = True
            else:
                toOrange.tap_ack = True
        #READYbLUE 
        if (fromBlue.mainPress and blueTapin): 
            toClock.readyBlue = 1 
            toBlue.main_ack = True
        #ReadyOrange
        if (fromOrange.mainPress and orangeTapin):
            toClock.readyOrange = 1  
            toOrange.main_ack = True

        #tapins are gui setup.

        # from fromclock.
        # done handleing. uh idk what that would entail but uh. 
        if fromClock.done:
            toClock.startClock = 0
        
        #TO button stuff? 
        #
        toBlue.color = 0
        toOrange.color = 0
        if(blueTapin == 1):
            if(toClock.readyBlue == 1):
                toBlue.color = 1
            else:
                toBlue.color = 2
        if(orangeTapin == 1 ):
            if(toClock.readyOrange ==1 ):
                toOrange.color = 1
            else:
                toOrange.color = 2
        
        
        if(toClock.win == 1 ):
            toOrange.color = 2
            toBlue.color = 3
        elif(toClock.win == 2 ):
            toOrange.color = 3
            toBlue.color = 2
        print(toOrange.color)
        toOrange.send()

        # should I put them to somehting other than the ready color when the match starts?

        #okay I think thats it... 

gui.self.impl.shutdown()
gui.glfw.terminate()