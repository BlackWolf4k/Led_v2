#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// For bzero()
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

// The principal six colors
// Green, Yellow, Red, Purple, Blue, Aqua
color_t colors[6] = { { 0, 255, 0 }, { 0, 255, 255 }, { 0, 0, 255 }, { 255, 0, 255 }, { 255, 0, 0 }, { 255, 255, 0 } };

// Numbers of colors
uint32_t number_of_colors = 6;

// Number of leds
uint32_t number_of_leds = 296;

#define OFFSET( x ) ( ( ( x % number_of_colors ) * number_of_leds ) / number_of_colors )

// Wich of R G B to change ( 0: R, 1: G, 2: B )
uint8_t value_to_change = 2;

int main()
{
	// Open the file
	FILE* file = NULL;
	file = fopen( "./rainbow.dat", "w" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		printf( "File opening error\n" );
		return 1;
	}

	// Create the animation file descriptor
	animation_file_descriptor_t animation_file_descriptor;
	animation_file_descriptor.delay = 100; // 100 ms
	animation_file_descriptor.repeat = 255; // loop
	animation_file_descriptor.line_length = number_of_leds * 3;
	animation_file_descriptor.number_of_lines = number_of_leds;

	// Write the animation file descriptor
	printf( "Writed %ld\n", fwrite( &animation_file_descriptor, sizeof( animation_file_descriptor_t ), 1, file ) );

	/* Generate the array of the colors that will be displayed */
	// Create the array of colors
	color_t* leds_colors = NULL;
	leds_colors = ( color_t* )calloc( number_of_leds, sizeof( color_t ) );

	// Check that the memory allocation was sucessfull
	if ( leds_colors == NULL )
	{
		printf( "Memory allocation error\n" );
		return 1;
	}

	// Set the colors at "pure" value
	for ( uint32_t color = 0; color < number_of_colors; color++ )
		leds_colors[ OFFSET( color ) ] = colors[ color ];

	// Store the values of the colors array
	for ( uint32_t color = 0; color < number_of_colors; color++ )
	{
		// Set how much one of the RGB colors has to grow
		uint8_t step = 255 / ( OFFSET( color + 1 ) - OFFSET( color ) - 1 );
		step = ( color % 2 == 0 ) ? step : -1 * step;

		// Set the other leds colors
		for ( uint32_t led = OFFSET( color ) + 1; led < OFFSET( color + 1 ); led++ )
		{
			// Copy the previous led values
			leds_colors[led] = leds_colors[ led - 1 ];
			// Change the color that has to change
			( ( uint8_t* )( &( leds_colors[led] ) ) )[value_to_change] += step;
		}
		// Change the next color to edit
		value_to_change = ( value_to_change + 3 - 1 ) % 3;
	}

	//for ( uint32_t i = 0; i < number_of_leds; i++ )
	//	printf( "%hhu, %hhu, %hhu\n", leds_colors[i].red, leds_colors[i].green, leds_colors[i].blue );

	// Save A LOT of calculations
	for ( uint32_t i = 0; i < animation_file_descriptor.number_of_lines; i++ )
	{
		printf( "Writed %ld\n", fwrite( leds_colors + i, sizeof( color_t ), number_of_leds - i, file ) );
		printf( "Writed %ld\n", fwrite( leds_colors, sizeof( color_t ), i, file ) );
	}

	// Close the file
	fclose( file );
	// Free the memory
	free( leds_colors );

	color_t color_read;
	file = fopen( "./rainbow.dat", "r" );
	fread( &animation_file_descriptor, sizeof( animation_file_descriptor_t ), 1, file );
	for ( uint32_t i = 0; i < animation_file_descriptor.number_of_lines * animation_file_descriptor.line_length / 3; i++ )
	{
		fread( &color_read, sizeof( color_t ), 1, file );
		printf( "%hhu, %hhu, %hhu\n", color_read.red, color_read.green, color_read.blue );
	}
}