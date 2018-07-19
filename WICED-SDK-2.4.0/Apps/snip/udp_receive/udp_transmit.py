#!/usr/bin/python

"""
A simple UDP transmit script demonstrating how to use
Python to send a UDP packet.

"""

import socket

# Address information of the target (use a broadcast address)
IPADDR = '192.168.0.255'
PORTNUM = 50007

# Data content of the UDP packet
PACKETDATA = 'Hello!'

# Initialize the socket (SOCK_DGRAM specifies that this is UDP)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)

# Connect the socket
s.connect((IPADDR, PORTNUM))

# Send the data
s.send(PACKETDATA)

# Close the socket
s.close()
