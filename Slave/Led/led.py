import time
import neopixel
from machine import Pin

from client import animation_descriptor_t
from shared_informations import server_callback, generals

leds = neopixel.NeoPixel( Pin( generals["control_pin"], Pin.OUT ), generals["number_of_leds"] )

# Play a animation
# Requires the animation and the animation descriptor as arguments
# Return a status code for certain cases ( 0: error, 1: success, 2: main server interrupted )
def play_animation( animation, animation_descriptor ):
	# Check wich animation playing algorithm to use
	if ( animation_descriptor.pattern == 1 ):
		play_rainbow_animation( animation, animation_descriptor )
	else:
		play_standard_animation( animation, animation_descriptor )

# Play a animation that follows the rainbow pattern
# Requires the animation and the animation descriptor as arguments
# Return a status code for certain cases ( 0: error, 1: success, 2: main server interrupted )
def play_rainbow_animation( animation, animation_descriptor ):
	global server_callback
	# Keep track of the times the leds have been changed
	turn = 0

	# Play the animation
	# Stop if the main server or there are no more animation repetitions
	while ( server_callback[0] == 0 and animation_descriptor.repeat > 0 ):
		# Decrease number of repetitions to do, unsless it is in loop mode
		if ( animation_descriptor.repeat != 255 ):
			animation_descriptor.repeat -= 1
		
		# Set all the leds colors
		for i in range( 0, generals["number_of_leds"], 1 ):
			leds[i] = ( animation[ 0 ][ ( ( i + turn ) % generals["number_of_leds"] ) * 3 + 0 ],
						animation[ 0 ][ ( ( i + turn ) % generals["number_of_leds"] ) * 3 + 1 ],
						animation[ 0 ][ ( ( i + turn ) % generals["number_of_leds"] ) * 3 + 2 ] )

		# Show the colors
		leds.write()

		# Sleep for the delay time
		time.sleep( animation_descriptor.delay / 1000 )

		turn += 1

	# What to return
	if ( server_callback[0] == 3 ): # The main server ordered to stop
		return 2
	else:
		return 1

# Play an animation that follow no standerd pattern
# Requires the animation and the animation descriptor as arguments
# Return a status code for certain cases ( 0: error, 1: success, 2: main server interrupted )
def play_standard_animation( animation, animation_descriptor ):
	global server_callback
	# Section of the animation to play
	animation_section = 0

	# Play the animation
	# Stop if the main server or there are no more animation repetitions
	while ( server_callback[0] == 0 and animation_descriptor.repeat > 0 ):
		# Encrease the stage of the animation
		animation_section = ( animation_section + 1 ) % animation_descriptor.number_of_lines
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
	
	# What to return
	if ( server_callback[0] == 3 ): # The main server ordered to stop
		return 2
	else:
		return 1