// General pourpose
#include <stdio.h>
#include <stddef.h>
#include "pico/stdlib.h"
// For the wifi connection
#include "pico/cyw43_arch.h"
// For the sockets
#include "lwip/sockets.h"
// For multi core
#include "pico/multicore.h"

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
struct animation_descriptor_t
{
	int number_of_lines;
	int line_length;
	uint8_t delay;
	uint8_t repeat;
	uint8_t* colors;
};

/*
SLAVE CONNECTION DESCRIPTOR
id: 			the id of the slave
ip_address: 	the ip address of the slave
*/
struct slave_connection_t
{
	uint8_t id;
	char ip_address[16];
};

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

// Set to 1 if the main server calls a forced stop
uint8_t forced_stop = 0;

int main()
{
	stdio_init_all();

	// Connect to the wifi
	if ( cyw43_arch_wifi_connect_timeout_ms( ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000 ) )
	{
		// There was an error connecting to the main server
		printf( "Failed to connect" );
		return 0; // Exit the program
	}
	// Connection was sucessfull
	printf( "Connected" );

	printf( "Starting callback server" );
	// Start the second core
	// The second core is a server that listens for main servers forced stop
	// multicore_launch_core1( &callback_server );

	uint8_t return_status = 0;

	slave_connection_t slave_informations;
	animation_descriptor_t animation_descriptor;

	// Create socket to connect to the main server
	int32_t socket_descriptor = 0;
	socket_descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	// Check that the creation of the socket was sucessfull
	if ( socket_descriptor < 0 )
	{
		printf( "There was an error" );
		return 0;
	}

	printf( "Connect to the main server" );
	// Connect to the main server and send basic informations
	return_status = send_slave_connection_informations( socket_descriptor, slave_informations );

	printf( "Recive the animation descriptor ");
	// Recive the animation descriptor
	return_status = recive_animation_descriptor( socket_descriptor, &animation_descriptor );

	printf( "Reciving the animation body" );
	// Recive the animation body
	return_status = recive_animation( socket_descriptor, &animation_descriptor );

	printf( "Starting to play the animation" );
	// Play the animation
	return_status = play_animation( socket_descriptor, &animation_descriptor );
}

uint8_t send_slave_connection_informations( int32_t socket_descriptor, slave_connection_t slave_informations )
{
	int8_t bytes_sent = 0;

	// Send the slave informations
	bytes_sent = send( socket_descriptor, *slave_informations, sizeof( slave_connection_t ), 0 );

	// Check that the sending was sucessfull
	if ( bytes_sent <= 0 )
		return 0;
	
	// Everyting was sucessfull
	return 1;
}

uint8_t recive_animation_descriptor( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{}

uint8_t recive_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{}

uint8_t play_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor )
{}