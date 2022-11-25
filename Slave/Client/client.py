# For sockets
import socket
# To handle structures easly
from ctypes import *

#SLAVE CONNECTION DESCRIPTOR
#id: 			the id of the slave
#ip_address: 	the ip address of the slave

# ANIMATION DESCRIPTOR
# number_of_lines: 	the number of lines in the file
# line_length:		the length of a single line
# repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
# delays: 			pointer to the delays matrix
# pattern:			animations with a repeating patter ( 0: none, 1: rainbow )

animation_descriptor_t = {
		"number_of_lines" : 0 | INT32,
		"line_length" : 4 | INT32,
		"delay" : 8 | UINT8,
		"repeat" : 9 | UINT8,
		"pattern" : 10 | UINT8
}

# Send the slave connection descriptor
# Requires as argument the socket with the main server, the id and the ip address
# Returns a status code ( 0: failure )
def send_connection_descriptor( socket_descriptor, slave_id, ip_address ):
	# Send the values
	bytes_sent = socket_descriptor.send( ( chr( slave_id ) + ip_address ).encode() )

	# Check that the sending was sucessfull
	# 8 is the minimun length of sendable informations
	if ( bytes_sent < 8 ):
		return 0 # There was an error while sending, return None

	# Everything went fine, return 1 as ok code
	return 1

# Recive the animation descriptor
# Requires as argument the socket with the main server
# Returns the recived animation descriptor in case of success, or 'None' in case of failure
def recive_animation_descriptor( socket_descriptor ):
	# Recive the buffer with animation descriptor
	buffer = socket_descriptor.recv( 13 )

	# Decode the recived buffer
	animation_descriptor = struct( addressof( buffer ), animation_descriptor_t, LITTLE_ENDIAN )

	# Check that the reciving was sucessfull
	if ( animation_descriptor.number_of_lines == 0 ):
		return None # There was and error while reciving, return None
	
	# Everything was fine, return the animation descriptor
	return animation_descriptor

# Recive the animation
# Requires as argument the socket with the main server and the animation descriptor
# Return a dictionary with the animation in case of success, or 'None' in case of failure
# This function has to be redesigned
def recive_animation( socket_descriptor, animation_descriptor ):
	# Create the animation dictionary
	animation = {}

	# Start reciving line by line
	for i in range( 0, animation_descriptor.number_of_lines, 1 ):
		animation[i] = b''

		# How many bytes where recived
		recived = 0

		# Recive the whole line
		while ( recived < animation_descriptor.line_length ):
			# Recive piece of animation
			buffer = socket_descriptor.recv( animation_descriptor.line_length + 1 )

			# Calculate bytes recived
			recived += len( buffer )

			# Append the animation piece
			animation[i] += buffer

	# Everything was fine, return the animation
	return animation