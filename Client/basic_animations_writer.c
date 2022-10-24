#include <stdio.h>
#include <stdint.h>

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} color_t;

typedef struct
{
	int number_of_lines;
	int line_length;
	uint8_t delay;
	uint8_t repeat;
} animation_descriptor_t;

int main()
{
	FILE* file = NULL;

	file = fopen( "../Server/animations_list.dat", "w" );

	uint32_t id = 0;

	printf( "Written: %d bytes\n" , fwrite( &id, sizeof( uint32_t ), 1, file ) );

	fprintf( file, "./animation__.dat" );

	/*animation_descriptor_t descriptor;
	descriptor.number_of_lines = 4;
	descriptor.line_length = 4 * 3;
	descriptor.delay = 10;
	descriptor.repeat = 255;

	printf( "Written: %d bytes\n" , fwrite( &descriptor, sizeof( descriptor ), 1, file ) );

	fputc( '\n', file );

	color_t led;

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	fputc( '\n', file );

	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	fputc( '\n', file );

	led.red = 255; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 0; led.green = 255; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 255; led.blue = 0;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );

	led.red = 255; led.green = 0; led.blue = 255;
	printf( "Written: %d bytes\n" , fwrite( &led, sizeof( led ), 1, file ) );*/


	fclose( file );
}