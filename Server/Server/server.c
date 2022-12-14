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

	// If the server crashed and tcp is still waiting to close socket, force the creation of a new one
	if ( setsockopt( socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &( int ){ 1 }, sizeof( int ) ) || setsockopt( socket_descriptor, SOL_SOCKET, SO_REUSEPORT, &( int ){ 1 }, sizeof( int ) ) )
		return 0;
	
	// Set timeout options for sending and reciving
	// If server gets in timeout it will suicide
	/*if ( setsockopt( socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &( struct timeval ){ 4 }, sizeof( struct timeval ) ) || setsockopt( socket_descriptor, SOL_SOCKET, SO_SNDTIMEO, &( struct timeval ){ 4 }, sizeof( struct timeval ) ) )
		return 0;*/

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
	server_address.sin_port = htons( port );

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

	// Create place where to store new socket
	int32_t* new_socket_descriptor = NULL;
	new_socket_descriptor = calloc( 1, sizeof( uint32_t ) );

	if ( new_socket_descriptor == NULL )
	{
		perror( "[Malloc Error]" );
		exit( 1 );
	}

	*new_socket_descriptor = 0;

	struct sockaddr_in address;
	socklen_t address_size = sizeof( address );

	printf( "Waiting connection\n" );

	// Accept the new connection
	*new_socket_descriptor = accept( *socket_descriptor, ( struct sockaddr* )&address, &address_size );

	printf( "Connection recived\n" );

	// Start the thread that handles the connection
	printf( "Socket: %d\n", *new_socket_descriptor);

	return pthread_create( &thread_id, NULL, handle_function, new_socket_descriptor );
}

void connection_error()
{
	perror( "An error has occurred\n[Socket Error]" ); // Print the occurred error
	exit( 1 );
}