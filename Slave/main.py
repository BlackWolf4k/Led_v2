import network
import socket
import time
# To handle structures easly

from Client.client import *
from Server.server import *
from Led.led import *


# SLAVE PHASES:
# 1. The slave connects to the main server sending the ip
# 2. The main server sends to the slave the descriptor of the animation to play
# 3. The main server sends to the slave the animation to play
# 4. The slave plays the animation
# 4. The slave checks if the main server is sending a force quit message
# 5. If a force quit message is sent, the slave repeats from 1
# 5. If the animation has ended the slave repeats from 1

generals = { "id" : 0, "number_of_leds": 150, "ssid" : "SmartLeds", "password" : "password", "client_port" : 1234, "server_port" : 1233 }

wlan = 0

def wifi_init():
	global wlan

	# Initialize the wireless connection
	wlan = network.WLAN( network.STA_IF )
	wlan.active( True )

	# Connect to the main server
	wlan.connect( generals["ssid"], generals["password"] )

	return

def slave_client():
	# Repeat untill the main server doesn't tell to turn off
	while ( server_callback != 2 ):
		# Create the socket
		socket_descriptor = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

		# Connect to the main server
		socket_descriptor.connect( ( wlan.ifconfig()[2], generals["client_port"] ) ) # Change this to wlan gateway ip

		# Send the slave connection descriptor
		status_code = send_connection_descriptor( socket_descriptor, generals["id"], wlan.ifconfig()[0] ) # Change this with wlan local ip

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
		status_code = play_animation( animation, animation_descriptor )

		# Main server ordered to turn off
		if server_callback == 2:
			return

	# Animation playing ended
	# Repeat

	return

def slave_server():
	start_server()
	return

# Main
if __name__ == '__main__':
	server_callback = False
	wifi_init()
	slave_client()
	slave_server()