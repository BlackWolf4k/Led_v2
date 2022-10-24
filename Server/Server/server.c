#include "server.h"
// For printf
#include <stdio.h>
// For type definitions
#include <stdlib.h>
// For threads
#include <pthread.h>

int32_t create_socket()
{
	// Store the socket descriptor
	int32_t socket_descriptor = 0;

	// Create the socket
	socket_descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	// Check if the creation of the socket was sucessfull
	if ( socket_descriptor < 0 )
		return 0; // Return error socket
	
	// Everything was fine, return the socket descriptor
	return socket_descriptor;
}

uint8_t bind_connection( int32_t* socket_descriptor, const char* ip, uint16_t port )
{
	int8_t status_code = 0;

	// Describe the server
	struct sockaddr_in server_address;

	// Initialize the server components
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr( ip );
	server_address.sin_port = port;

	// Bind the connection
	status_code = bind( *socket_descriptor, ( struct sockaddr* )&server_address, sizeof( server_address ) );

	// Check that the binding was sucessfull
	if ( status_code < 0 )
		return 0; // Thre was an error

	// Everything was fine
	return 1;
}

uint8_t listen_connection( int32_t* socket_descriptor, uint32_t maximum_connections )
{
	int8_t status_code = 0;

	// Start listening
	status_code = listen( *socket_descriptor, maximum_connections );

	// Check that the start of listening was sucessfull
	if ( status_code < 0 )
		return 0;
	
	// Everything was fine
	return 1;
}

pthread_t accept_connection( int32_t* socket_descriptor, void*( *handle_function )( void* ) )
{
	pthread_t thread_id = 0;

	int32_t new_socket_descriptor = 0;
	struct sockaddr_in address;
	socklen_t address_size = sizeof( address );

	printf( "Waiting connection" );

	// Accept the new connection
	new_socket_descriptor = accept( *socket_descriptor, ( struct sockaddr* )&address, &address_size );

	printf( "Connection recived" );

	// Start the thread that handles the connection
	pthread_create( &thread_id, NULL, handle_function, &new_socket_descriptor );

	return 1;
}

void connection_error()
{
	perror( "An errros has occurred" ); // Print the occurred error
	exit( 0 );
}