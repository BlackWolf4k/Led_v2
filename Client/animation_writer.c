#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// For bzero
#include <string.h>

/*
ANIMATION FILE DESCRIPTOR
number_of_line: 	the number of lines in the file
line_length:		the length of a single line
delay:				the delay between each change of led colors
repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
*/
typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t delay;
	uint8_t repeat;
} animation_file_descriptor_t;

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} color_t;

int main()
{
	animation_file_descriptor_t animation_file_descriptor;

	uint32_t number_of_leds = 0;

	FILE* file = NULL;
	file = fopen( "./animation.dat", "w" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		// There was an error while opening the file, exit the program
		printf( "Error while opening the animation file\n" );
		return 1;
	}

	// Insert number of leds
	printf( "Insert the number of leds:\n" );
	scanf( "%u", &number_of_leds );

	// Insert number of animation phases
	printf( "Insert number of phases of the animation\n" );
	scanf( "%u", &animation_file_descriptor.number_of_lines );

	// Insert repetitions
	printf( "Insert number of repetitions of the animation ( 255 = loop )\n" );
	scanf( "%hhu", &animation_file_descriptor.repeat );

	// Inser delay
	printf( "Insert delay of each phase of the animation\n" );
	scanf( "%hhu", &animation_file_descriptor.delay );

	// Calculate the length of each line
	animation_file_descriptor.line_length = number_of_leds * sizeof( color_t );

	// Write the animation file descriptor
	// Check that the writing was sucessfull
	printf( "Writed %ld bytes\n", fwrite( &animation_file_descriptor, sizeof( animation_file_descriptor_t ), 1, file ) );
	/*if ( fwrite( &animation_file_descriptor, sizeof( animation_file_descriptor_t ), 1, file ) < sizeof( animation_file_descriptor_t ) - 1 )
	{
		// There was an error, exit the program
		printf( "Error while writing the animation descriptor\n" );
		return 1;
	}*/

	// Buffer for each line
	color_t* leds = NULL;
	leds = ( color_t* )calloc( number_of_leds, sizeof( color_t ) );

	// Check that the memory allocation was sucessfull
	if ( leds == NULL )
		return 1; // There was an error, return
	
	// Some general information
	uint8_t go = 1;
	uint32_t led_start = 0;
	uint32_t led_end = 0;
	color_t color;

	// Get each phases colors
	for ( uint32_t i = 0; i < animation_file_descriptor.number_of_lines; i++ )
	{
		go = 1;

		// Repeat untill all leds colors are inserted
		while ( go )
		{
			// Insert leds to change color
			do
			{
				printf( "Insert led start and end\n" );
				scanf( "%u %u", &led_start, &led_end );
			} while ( !( led_start <= number_of_leds && led_end <= number_of_leds ) );

			// Insert color for leds
			printf( "Inser the color for the group of leds [ R G B ]\n" );
			scanf( "%hhu %hhu %hhu", &color.red, &color.green, &color.blue );

			// Copy the colors for the leds
			for ( uint32_t i = led_start; i < led_end; i++ )
				leds[i] = color;

			// Ask to keep going
			printf( "Do you want to keep on going? ( 0 - 1 )\n" );
			scanf( "%hhu", &go );
		}
		// Write the animation phases
		// Check that the writing was sucessfull
		printf( "Writed %ld bytes\n", fwrite( leds, number_of_leds * sizeof( color_t ), 1, file ) );
		/*if ( fwrite( leds, number_of_leds * sizeof( color_t ), 1, file ) < number_of_leds * sizeof( color_t ) )
		{
			// There was an error, exit the program
			printf( "Error while writing animation\n" );
			return 1;
		}*/
	}

	return 0;
}