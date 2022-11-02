#include <stdio.h>

#include "pico/stdlib.h"
// Wireless driver
#include "pico/cyw43_arch.h"
// For tcp comunication
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "./Client/client.h"

// Start the connection with the main server
// Send basic informations about the slave
// Requires the slave informations and the socket where to send data
// Return a status code where the sending was sucessfull ( 0: error )
uint8_t send_slave_connection_informations( int32_t socket_descriptor, slave_connection_t slave_informations );

// Recive the animation descriptor from the main server
// Requires a argument where to store the animation descriptor and the socket where to send data
// Return a status code where the reciving was sucessfull ( 0: error )
uint8_t recive_animation_descriptor( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Recive the animation colors and delays from the main server
// Requires the animation descriptor as argument and the socket where to send data
// Return a status code where the reciving was sucessfull ( 0: error )
uint8_t recive_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Play the animation
// Requires as argument the animation descriptor and the socket where to sand data
// Returns a status code ( 0: animation ended, 1: main server forced to quit, 0: error )
uint8_t play_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Check if the main server calls a forced stop
// The value at 0x is set to 1 for forced stop, else 0
// Requires no argument
// Returns nothing
void callback_server();

int main()
{
	stdio_init_all();

	// Initialize the wifi
	cyw43_arch_init();

	// Connect to the server access point
	if ( cyw43_arch_wifi_connect_timeout_ms( WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000 ) )
	{
		printf( "There was an error while connecting to the server\n" );
		return 1;
	}
	printf( "Connection sucessfull\n" );

	client_t* client = client_init();

	send( client );

	// Stop the wifi
	cyw43_arch_deinit();
}

uint8_t send_slave_connection_informations( int32_t socket_descriptor, slave_connection_t slave_informations )
{
	int8_t bytes_sent = 0;

	// Send the slave informations
	bytes_sent = send( socket_descriptor, &slave_informations, sizeof( slave_connection_t ), 0 );

	// Check that the sending was sucessfull
	if ( bytes_sent <= 0 )
		return 0;
	
	// Everyting was sucessfull
	return 1;
}

uint8_t recive_animation_descriptor( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{
	// Allocate the buffer to store the messages
	uint8_t* buffer = NULL;
	buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the buffer allocation was sucessfull
	if ( buffer == NULL )
		return 0; // There was an error while allocating the buffer
	
	uint32_t bytes_recived = 0;

	// Recive the animation descriptor
	bytes_recived = recv( socket_descriptor, buffer, BUFFER_SIZE, 0 );
	perror( "[Socket Error]" );

	// Check that the animation descriptor reciving was sucessfull
	// Check that the reciving was sucessfull
	if ( bytes_recived <= sizeof( animation_descriptor_t ) )
	{
		printf( "Recived: %d\n", bytes_recived );
		return 0; // There was an error while reciving
	}
	
	// Store the animation descriptor in the passed argument
	*animation_descriptor = *( ( animation_descriptor_t* )( buffer ) );

	// Everything went fine
	// Return ok status code
	return 1;
}

uint8_t recive_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{
	// Allocate the buffer for the animation
	animation_descriptor -> animation = NULL;
	animation_descriptor -> animation = ( uint8_t* )calloc( animation_descriptor -> line_length * animation_descriptor -> number_of_lines, sizeof( uint8_t ) );

	// Allocate the buffer for the trasmission
	uint8_t* buffer = NULL;
	buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the buffers allocation was sucessfull
	if ( animation_descriptor -> animation == NULL || buffer == NULL )
		return 0; // Something went wrong in the allocation of the buffers
	
	// Store informations about the segments of trasmission recived
	uint32_t bytes_recived_total = 0;
	uint32_t bytes_recived = 0;

	// Start reciving the animation
	for ( uint32_t i = 0; i < animation_descriptor -> number_of_lines; i++ )
	{
		bytes_recived_total = 0;
		bytes_recived = 0;

		// Recive line
		while ( bytes_recived_total < animation_descriptor -> line_length )
		{
			// Recive the part of animation
			bytes_recived = recv( socket_descriptor, buffer, BUFFER_SIZE, 0 );

			// Check that the reciving was sucessfull
			if ( bytes_recived <= 0 )
			{
				perror( "[Socket Error]" );
				return 0; // There was an error in the trasmission
			}

			bytes_recived_total += bytes_recived;

			// Store what recived in the animation buffer
			memcpy( animation_descriptor -> animation + i * animation_descriptor -> line_length, buffer, bytes_recived );

			// Clear the buffer
			bzero( buffer, BUFFER_SIZE );
		}
	}
}

uint8_t play_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{
	printf( "Number of Lines:%d, Line Length:%d, Repetitions:%d, Delay: %d\n", animation_descriptor -> number_of_lines, animation_descriptor -> line_length, animation_descriptor -> repeat, animation_descriptor -> delay );
	printf( "Animaton body:\n" );
	for ( int i = 0; i < animation_descriptor -> line_length * animation_descriptor -> number_of_lines; i++ )
	{
		printf( "%u ", animation_descriptor -> animation[i] );
		if ( i % 3 == 0 )
			printf( "\n" );
	}
}

void callback_server();