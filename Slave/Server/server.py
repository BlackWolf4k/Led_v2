# For sockets
import socket
# To handle structures easly
from ctypes import *

from main import wlan, generals

# Value set by main server
# 0: go ,1: get new animation, 2: turn off
server_callback = 0

def start_server():
	global server_callback
	global wlan

	# Createthe socket descriptor
	socket_descriptor = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

	# Bind the connection
	socket_descriptor.bind( ( wlan.ifconfig()[0], generals["server_port"] ) ) # Replace ip with wlan ip

	# Start to listen on the connection
	socket_descriptor.listen()

	# Accept connection untill the server doesn't ask to turn off
	while ( server_callback != 2 ):
		# Accept the connection
		connection, address = socket_descriptor.accept()

		# Check if connected
		with connection:
			server_callback = int( socket_descriptor.recv( 1 ).decode(), base = 16 )