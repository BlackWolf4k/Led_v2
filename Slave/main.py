# import network
import socket
import time


# SLAVE PHASES:
# 1. The slave connects to the main server sending the ip
# 2. The main server sends to the slave the descriptor of the animation to play
# 3. The main server sends to the slave the animation to play
# 4. The slave plays the animation
# 4. The slave checks if the main server is sending a force quit message
# 5. If a force quit message is sent, the slave repeats from 1
# 5. If the animation has ended the slave repeats from 1

# ANIMATION DESCRIPTOR
# number_of_lines: 	the number of lines in the file
# line_length:		the length of a single line
# repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
# colors: 			pointer to the colors matrix
# delays: 			pointer to the delays matrix

class animation_descriptor:
	# Load from the buffer of the connection the animation descriptor informations
	def load_descriptor( self, buffer ):
		self.number_of_lines = buffer[ 0 : 4 ] # First four bytes, integer size
		self.line_length = buffer[ 4 : 8 ] # Second four bytes, integer size
		self.repeat = buffer[ 8 : 9 ] # Signles byte
		self.delays = buffer[ 9 : 10 ] # Single byte
		return

# Start the connection with the main server
# Send basic informations about the slave
# Requires the slave informations and the socket where to send data
# Return a status code where the sending was sucessfull ( 0: error )
def send_slave_connection_informations( socket_descriptor, slave_informations ):
	return 1

# Recive the animation descriptor from the main server
# Requires a argument where to store the animation descriptor and the socket where to send data
# Return a status code where the reciving was sucessfull ( 0: error )
def recive_animation_descriptor( socket_descriptor, animation_descriptor ):
	return 1

# Recive the animation colors and delays from the main server
# Requires the animation descriptor as argument and the socket where to send data
# Return a status code where the reciving was sucessfull ( 0: error )
def recive_animation( socket_descriptor, animation_descriptor ):
	return 1

# Play the animation
# Requires as argument the animation descriptor and the socket where to sand data
# Returns a status code ( 0: animation ended, 1: main server forced to quit, 0: error )
def play_animation( socket_descriptor, animation_descriptor ):
	return 1

# Check if the main server calls a forced stop
# The value at 0x is set to 1 for forced stop, else 0
# Requires no argument
# Returns nothing
def callback_server():
	return 1