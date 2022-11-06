import time
from neopixel import Neopixel

from Server.server import server_callback
from Client.client import animation_descriptor_t

number_of_leds = 150

control_pin = 0

leds = Neopixel( number_of_leds, 0, control_pin, "RGB" )

# Play a animation
# Requires the animation and the animation descriptor as arguments
# Return a status code for certain cases ( 0: error, 1: success, 2: main server interrupted )
def play_animation( animation, animation_descriptor ):
	# Play the animation
	# Stop if the main server or there are no more animation repetitions
	while ( server_callback == 0 and animation_descriptor.repeat > 0 ):
		# Decrease number of repetitions to do, unsless it is in loop mode
		if ( animation_descriptor.repeat != 255 ):
			animation_descriptor.repeat -= 1
		
		# Set all the leds colors
		for i in range( 0, number_of_leds, 1 ):
			leds.set_pixel( i, ( animation[ i * 3 + 0 ], animation[ i * 3 + 1 ], animation[ i * 3 + 2 ] ) )
		
		# Show the colors
		leds.show()

		# Sleep for the delay time
		time.sleep( animation_descriptor.delay / 1000 )
	return 1