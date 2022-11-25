# For sockets
import socket
# To handle structures easly
from ctypes import *

from main import wlan
from shared_informations import generals, server_callback

def start_server():
	global server_callback
	global wlan

	# Createthe socket descriptor
	socket_descriptor = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

	# Bind the connection
	socket_descriptor.bind( ( wlan.ifconfig()[0], generals["server_port"] ) ) # Replace ip with wlan ip

	# Start to listen on the connection
	socket_descriptor.listen( 2 )

	# Accept connection untill the server doesn't ask to turn off
	while ( server_callback != 3 ):
		# Accept the connection
		connection, address = socket_descriptor.accept()

		# Check if connected
		with connection:
			# Recive the connection
			buffer = connection.recv( 1024 )

			# Send a ok response
			connection.send( "HTTP/1.0 200 OK\r\n".encode() )
			connection.close()
	# Exit
	return