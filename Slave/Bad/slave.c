// General pourpose
#include <stdio.h>
#include <stddef.h>
// For types definitions
#include <stdint.h>
// For memcpy and bzero
#include <string.h>
/*
#include "pico/stdlib.h"
// For the wifi connection
#include "pico/cyw43_arch.h"
// For the sockets
#include "lwip/sockets.h"
#include "lwip/inet.h"
// For multi core
#include "pico/multicore.h"
*/

/*REMOVE THIS TO RUN ON PICO*/
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
SLAVE PHASES:
1. The slave connects to the main server sending the ip
2. The main server sends to the slave the descriptor of the animation to play
3. The main server sends to the slave the animation to play
4. The slave plays the animation
4. The slave checks if the main server is sending a force quit message
5. If a force quit message is sent, the slave repeats from 1
5. If the animation has ended the slave repeats from 1
*/

/*
ANIMATION DESCRIPTOR
number_of_line: 	the number of lines in the file
line_length:		the length of a single line
repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
colors: 			pointer to the colors matrix
delays: 			pointer to the delays matrix
*/
typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t delay;
	uint8_t repeat;
	uint8_t* animation;
} animation_descriptor_t;

/*
SLAVE CONNECTION DESCRIPTOR
id: 			the id of the slave
ip_address: 	the ip address of the slave
*/
typedef struct
{
	uint8_t id;
	char ip_address[16];
} slave_connection_t;

#define BUFFER_SIZE 1024

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

const char* ssid = "SmartLeds";
const char* password = "password";
const char* main_server_address = "192.136.60.51";

// Set to 1 if the main server calls a forced stop
uint8_t forced_stop = 0;

int main()
{
/*	stdio_init_all();

	// Connect to the wifi
	if ( cyw43_arch_wifi_connect_timeout_ms( ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000 ) )
	{
		// There was an error connecting to the main server
		printf( "Failed to connect" );
		return 0; // Exit the program
	}
	// Connection was sucessfull
	printf( "Connected" );

	printf( "Starting callback server\n" );
	// Start the second core
	// The second core is a server that listens for main servers forced stop
	// multicore_launch_core1( &callback_server );
*/
	uint8_t return_status = 0;

	slave_connection_t slave_informations;
	animation_descriptor_t animation_descriptor;

	slave_informations.id = 0;
	strcpy( slave_informations.ip_address, "255.255.255.255" );

	// Create socket to connect to the main server
	int32_t socket_descriptor = 0;
	socket_descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr( main_server_address );
	server_address.sin_port = 1234;

	if ( connect( socket_descriptor, ( struct sockaddr* )&server_address, sizeof( server_address ) ) == -1 )
	{
		printf( "Could not connect to main server\n" );
		return 0;
	}

	// Check that the creation of the socket was sucessfull
	if ( socket_descriptor < 0 )
	{
		printf( "There was an error\n" );
//		cyw43_arch_deinit();
		return 0;
	}

	printf( "Connect to the main server\n" );
	// Connect to the main server and send basic informations
	return_status = send_slave_connection_informations( socket_descriptor, slave_informations );

	if ( !return_status )
	{
		printf( "There was an error while sending basic informations\n" );
//		cyw43_arch_deinit();
		return 0;
	}

	printf( "Recive the animation descriptor\n" );
	// Recive the animation descriptor
	return_status = recive_animation_descriptor( socket_descriptor, &animation_descriptor );

	if ( !return_status )
	{
		printf( "There was an error while recivig the animation descriptor\n" );
//		cyw43_arch_deinit();
		return 0;
	}

	printf( "Reciving the animation body\n" );
	// Recive the animation body
	return_status = recive_animation( socket_descriptor, &animation_descriptor );

	printf( "Starting to play the animation\n" );
	// Play the animation
	return_status = play_animation( socket_descriptor, &animation_descriptor );

//	cyw43_arch_deinit();
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
