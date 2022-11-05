# import network
import socket
import time
# To handle structures easly

from Client.client import *
import Server


# SLAVE PHASES:
# 1. The slave connects to the main server sending the ip
# 2. The main server sends to the slave the descriptor of the animation to play
# 3. The main server sends to the slave the animation to play
# 4. The slave plays the animation
# 4. The slave checks if the main server is sending a force quit message
# 5. If a force quit message is sent, the slave repeats from 1
# 5. If the animation has ended the slave repeats from 1

id = 0

def slave_client():
	# Create the socket
	socket_descriptor = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

	# Connect to the main server
	socket_descriptor.connect( ( "127.0.0.1", 53764 ) ) # Change this to wlan gateway ip

	# Send the slave connection descriptor
	status_code = send_connection_descriptor( socket_descriptor, id, "255.255.255.255" ) # Change this with wlan local ip

	# Check that the sending of the connection descriptor returned a success value
	if ( not status_code ):
		print( "Error" )
		return

	# Recive the animation descriptor
	animation_descriptor = recive_animation_descriptor( socket_descriptor )

	# Check that the reciving of the animation descriptor was not None
	if ( animation_descriptor == None ):
		print( "Error" )
		return
	
	# Recive the animation
	animation = recive_animation( socket_descriptor, animation_descriptor )

	# Check that the reciving of the animation was not None
	if ( animation == None ):
		print( "Error" )
		return
	
	print( animation )
	
	# Play the animation

	# Animation playing ended
	# Repeat

	return

def slave_server():
	return

# Main

if __name__ == '__main__':
	slave_client()