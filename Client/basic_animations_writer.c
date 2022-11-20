#include <stdio.h>
#include <stdint.h>

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} color_t;

/*
ANIMATION FILE DESCRIPTOR
number_of_line: 	the number of lines in the file
line_length:		the length of a single line
delay:				the delay between each change of led colors
repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
pattern:			animations with a repeating patter ( 0: none, 1: rainbow )
*/
typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t delay;
	uint8_t repeat;
	uint8_t pattern;
} animation_file_descriptor_t;

int main()
{
	FILE* file = NULL;

	file = fopen( "../Server/execution_space/animation__.dat", "w" );

	uint32_t id = 0;

	animation_file_descriptor_t descriptor;
	descriptor.number_of_lines = 4;
	descriptor.line_length = 4 * 3;
	descriptor.delay = 10;
	descriptor.repeat = 255;
	descriptor.pattern = 0;

	printf( "Written: %d\n" , fwrite( &descriptor, sizeof( descriptor ), 1, file ) );

	color_t led;

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );


	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );


	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d\n" , fwrite( &led, sizeof( led ), 1, file ) );


	fclose( file );
}