
import socket
import struct
import time
import pygame
import messages
import gui
import time
import sys
import errno
import os
import serial
import threading

Gui = gui.GUI()

printOrange = 0
printBlue = 0
printClock = 0

done = False

ser = serial.Serial('COM4', 115200) 
ser.set_buffer_size(1024)

# I 'can' only send on one port, so i need to differectiate between goes to clock and  goes to button.   I thing data << 1 + 1 or +0, then check %2, on the arduino end to see.

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

#region socks
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


#endregion socks


# color key: -1 = off, 0 = standby color, 1 = ready color(orange, blue), 2 = win color, 3 = loose color (Red, probably)


def parseData( data ): # im just gonna add an int to all of these
    #print(data)
        for i in range(len(data)):
            if(data[i] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']):

                data[i] = int(data[i])

        if(data[0] == 0x1):
            #print("fromClock")
            fromClock.fromBytes(data)
            return 1
        elif(data[0] == 3):
            fromOrange.fromBytes(data)
            return 1
        elif(data[0] == 2):
            #print(data[1])
    
            #print("FromBlue")
            print("fromOrange", data)

            fromBlue.fromBytes(data)
            return 1
        else:
            return 0

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

    if ( Gui.blueTap == 1 or
         Gui.orangeTap == 1 or
         Gui.start == 1 or
         Gui.reset == 1 or
         Gui.pause == 1 ):
        
        buf =  Gui.reset*2**6 + Gui.pause*2**5 + Gui.orangeTap*2**4 + Gui.blueTap*2**3+ Gui.start*2**2
        print(hex(buf))
        buf = hex(buf)
        buf += "\n"
        ser.write(buf.encode())

        Gui.blueTap = 0
        Gui.orangeTap = 0
        Gui.start = 0
        Gui.reset = 0
        Gui.pause = 0
    #received_data = ser.readline().decode('utf-8').strip()
    #if received_data:
    #    print(f"Received: {received_data}")

    
    #if (time.time() - fastTimer) > 0.3: # in sec
    #    # WRITE:
    #    fastTimer = time.time()
    #    #print("sending things")
    #    toOrange.send()
    #    toBlue.send()
    #    toClock.send()

        #printing routine. 

        #ser.write(thing.encode())
                

        
    #try:
    #    indata = rx_sock.recv(1024)
    #    #print("after")
    #    
    #except socket.error as e:
    #    err = e.args[0]
    #    e.args[0]
    #    if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
    #        #print ('No data available') 
    #        pass 
    #    else: # a "real" error occurred print
    #        sys.exit(1) 
    #else: # got a message
    #    #print("DATATATAT")   
    #    if(len(indata) > 1):
    #        parseData( indata ) 
    #print("hi?")

    #print(len(indata))
    
        #print("parsisisnd")

    # Check changes and set acks?
    
    # TOCLCOK
    #3start int gui
    #3reset in gui
    #3pause in gui
    #win can be tapout ( 1= orange win)
    #if(fromBlue.tapoutPress == 1 or fromOrange.tapoutPress == 1):
    #    if fromBlue.tapoutPress == 1:
    #        print("from Blue TAPPOUE")
    #        toClock.win = 2
    #    elif fromOrange.tapoutPress == 1:
    #        print("FROM ORANGE ")
    #        toClock.win = 1
#
#
    #    if toClock.win == 1:
    #        toBlue.tap_ack = True
    #    else:
    #        toOrange.tap_ack = True
    ##print(toClock.reset, fromClock.reset_ack)
    #if(toClock.reset == 1 and fromClock.reset_ack ):
    #    toClock.reset = 0
#
    ##READYbLUE 
    #if (fromBlue.mainPress == 1 and toClock.blueTapin == 1):
    #    print("ASSSSSSSSSS") 
    #    toClock.readyBlue = 1 
    #    toBlue.main_ack = True
    #elif (fromBlue.mainPress == 1):
    #    print("BBBBBBBBBBBBBBBBBBBBB")
    #    toBlue.main_ack = True
    #else:
    #    #print("CCCCCCCCCCCCCCCs")
    #    toBlue.main_ack = False
#
    ##ReadyOrange
    #if (fromOrange.mainPress == 1 and toClock.orangeTapin == 1):
    #    print("ASSSSSSSSSS") 
#
    #    toClock.readyOrange = 1  
    #    toOrange.main_ack = True
    #elif ( fromOrange.mainPress == 1):
    #    print("BBBBBBBBBBBBBBBBBBBBB")
#
    #    toOrange.main_ack = True
    #else:
    #    #print("CCCCCCCCCCCCCCCs")
#
    #    toOrange.main_ack = False
#
#
    ##tapins are gui setup.
#
    ## from fromclock.
    ## done handleing. uh idk what that would entail but uh. 
    #if fromClock.done == 1:
    #    toClock.startClock = 0
    #    #while True:
    #    #    print("DONNNNNE")
    #
    ##TO button stuff? 
    ##
    #toBlue.color = 0
    #toOrange.color = 0
    #
    #if(toClock.startClock == 1):
    #    toBlue.color = 1
    #if(blueTapin == 1):
    #    if(toClock.readyBlue == 1):
    #        toBlue.color = 1
    #    else:
    #        toBlue.color = 2
#
    #if(toClock.startClock == 1):
    #    toOrange.color = 1
    #elif(orangeTapin == 1 ):
    #    if(toClock.readyOrange ==1 ):
    #        toOrange.color = 1
    #    else:
    #        toOrange.color = 2
    #
    #
    #if(toClock.win == 1 ):
    #    print("this ine") # orange win
    #    toOrange.color = 3
    #    toBlue.color = 2
    #elif(toClock.win == 2 ):
    #    toOrange.color = 2
    #    toBlue.color = 3
#
#
    ## should I put them to somehting other than the ready color when the match starts?
#
    ##okay I think thats it... 
    ##print("SJDJSJJ")
    #s = ""
    #if printOrange == 1:
    #    s  += "(Orange) Color: " + str(toOrange.color) + " | TAP_ACK: " + str(toOrange.tap_ack) + " | main: " + str(toOrange.main_ack) + "\n"
    #if printBlue == 1:
    #    s += "(Blue) Color: " + str(toBlue.color) + " | TAP_ACK: " + str(toBlue.tap_ack) + " | main: " + str(toBlue.main_ack) + "\n"
    #if printClock == 1:
    #    s += "(Clock) Start: " + str(toClock.startClock) + " | rBlue: " + str(toClock.readyBlue) + " | rOrange: " + str(toClock.readyOrange) + " | pause: " + str(toClock.pause) + " | reset: " + str(toClock.reset) # theres more but il add tase if nessisary
    #if len(s) > 1:
    #    #os.system('cls')
    #    print(s)
    #    #print(fromBlue.mainPress)
done = True
Gui.impl.shutdown()

gui.glfw.terminate()




#
## how will I encode that. nasicaly I need any buttion pushed that I could do.
# so tapins (2) and start, reset, pause, KO button(2?)
# so 7 total. uh could do it in one byte, so 03\n is 0000011 or whatever. 
# annd I do 0 - OKO - BKO - Otap - Btap - pause - reset - start.
##
#
#
#