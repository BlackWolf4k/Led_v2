import time
import neopixel
from machine import Pin

from Server.server import server_callback
from Client.client import animation_descriptor_t
from main import generals

# Control Pin is GPIO 0
control_pin = 0

leds = neopixel.NeoPixel( Pin( control_pin, Pin.OUT ), generals["number_of_leds"] )

# Play a animation
# Requires the animation and the animation descriptor as arguments
# Return a status code for certain cases ( 0: error, 1: success, 2: main server interrupted )
def play_animation( animation, animation_descriptor ):
	# Section of the animation to play
	animation_section = 0

	# Play the animation
	# Stop if the main server or there are no more animation repetitions
	while ( server_callback == 0 and animation_descriptor.repeat > 0 ):
		animation_section += 1
		# Decrease number of repetitions to do, unsless it is in loop mode
		if ( animation_descriptor.repeat != 255 ):
			animation_descriptor.repeat -= 1

		# Set all the leds colors
		for i in range( 0, generals["number_of_leds"], 1 ):
			leds[i] = ( animation[ animation_section ][ i * 3 + 0 ], animation[ animation_section ][ i * 3 + 1 ], animation[ animation_section ][ i * 3 + 2 ] )

		# Show the colors
		leds.write()

		# Sleep for the delay time
		time.sleep( animation_descriptor.delay / 1000 )
	return 1