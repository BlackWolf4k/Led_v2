# For sockets
import socket
# To handle structures easly
from ctypes import *

#SLAVE CONNECTION DESCRIPTOR
#id: 			the id of the slave
#ip_address: 	the ip address of the slave

class slave_descriptor_t( Structure ):
	_fields_ = [ ( 'id', c_uint8 ), ( 'ip_address', c_char * 16 ) ]

# ANIMATION DESCRIPTOR
# number_of_lines: 	the number of lines in the file
# line_length:		the length of a single line
# repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
# colors: 			pointer to the colors matrix
# delays: 			pointer to the delays matrix

class animation_descriptor_t( Structure ):
	_fields_ = [ ( 'number_of_lines', c_uint32 ), ( 'line_length', c_uint32 ), ( 'delay', c_uint8 ), ( 'repeat', c_uint8 ) ]

# Send the slave connection descriptor
# Requires as argument the socket with the main server, the id and the ip address
# Returns a status code ( 0: failure )
def send_connection_descriptor( socket_descriptor, slave_id, ip_address ):
	# Create the slave descriptor
	slave_descriptor = slave_descriptor_t()

	# Set the values
	slave_descriptor.id = slave_id
	slave_descriptor.ip_address = ip_address.encode() # Encode for the sending

	# Send the values
	bytes_sent = socket_descriptor.send( slave_descriptor )

	# Check that the sending was sucessfull
	if ( bytes_sent < 17 ):
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
	animation_descriptor = animation_descriptor_t.from_buffer_copy( buffer )

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

		# Recive piece of animation
		buffer = socket_descriptor.recv( animation_descriptor.line_length )

		# Check that the reciving was sucessfull
		# if ( buffer.len <= animation_descriptor.line_length ): # or buffer.len > animation_descriptor.line_length ):
		# 	return None # There was and error while reciving, return None

		# Check if recived the whole animation line
		# if ( buffer.len < animation_descriptor.line_length )
		# Append the pice to the line
		# Append the line
		animation[i] += buffer

	# Everything was fine, return the animation
	return animation