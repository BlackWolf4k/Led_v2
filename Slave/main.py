import network
import socket
import time
# To handle structures easly

from server import *
from client import *
from led import *
from shared_informations import *

# To use both cores
import _thread


# SLAVE PHASES:
# 1. The slave connects to the main server sending the ip
# 2. The main server sends to the slave the descriptor of the animation to play
# 3. The main server sends to the slave the animation to play
# 4. The slave plays the animation
# 4. The slave checks if the main server is sending a force quit message
# 5. If a force quit message is sent, the slave repeats from 1
# 5. If the animation has ended the slave repeats from 1

wlan = network.WLAN( network.STA_IF )

def wifi_init():
	global wlan

	# Initialize the wireless connection
	wlan.active( True )

	# Connect to the main server
	wlan.connect( generals["ssid"], generals["password"] )

	return

# Restart the wifi connection with the router
# If not done may get some connection problems
def restart_wifi():
	# Connect to the main server
	wlan.connect( generals["ssid"], generals["password"] )

def get_animation():
	global server_callback

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
	
	# Play the animation
	status_code = play_animation( animation, animation_descriptor )

	# Delay to avoid server over requesting
	time.sleep( 5 )

	return

def slave_client():
	global server_callback

	# Repeat untill the main server doesn't tell to turn off
	while ( server_callback != 3 and server_callback != 2 ):
		# Sleep different times bases on server commands
		if ( server_callback == 3 ):
			time.sleep( 16 )
		elif ( server_callback == 2 ):
			time.sleep( 1 )

		# Try to connect to the main server
		try:
			get_animation()
		# Except a connection error
		except OSError:
			print( "Host unreach" )
			# Wait some time before making another request
			time.sleep( 5 )

def slave_server():
	start_server()
	return

# Main
if __name__ == '__main__':
	global server_callback

	# Start everything
	server_callback = False
	wifi_init()

	# Start the two main function in the two different cores
	client_thread = _thread.start_new_thread( slave_client, () )

	slave_server()