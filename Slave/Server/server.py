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

	# Override the address beeing used
	socket_descriptor.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )

	# Bind the connection
	socket_descriptor.bind( ( "0.0.0.0", generals["server_port"] ) ) # Replace ip with wlan ip

	# Start to listen on the connection
	socket_descriptor.listen( 2 )

	print( "Started Server" )

	# Accept connection untill the server doesn't ask to turn off
	while ( server_callback[0] != 3 ):
		print( "Started to accept connections" )

		# Accept the connection
		connection, address = socket_descriptor.accept()

		print( "Connected to:" + str( address ) )

		# Recive the connection
		request = connection.recv( 1024 )

		response = ""

		# Check what has bee asked
		if ( generals["slave_password"] in request.decode() ):
			response = "Shutting down slave"
			server_callback[0] = 3
		elif ( "freeze" in request.decode() ):
			if ( server_callback[0] == 2 ):
				response = "UnFrozen"
				server_callback[0] = 0
			elif ( server_callback[0] != 3 ):
				response = "Frozen"
				server_callback[0] = 2

		# Send a ok response
		connection.send( "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n" )
		connection.send( "<!DOCTYPE html><html>" + response + "</html>" )
		connection.close()

	# Exit
	print( "Shutting down slave client" )
	return