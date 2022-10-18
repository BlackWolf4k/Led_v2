#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
// Socket
#include <sys/socket.h>
#include <arpa/inet.h>
// Fork
#include <unistd.h>
#include <sys/wait.h>

#include "./slave.h"

#define MAX_NUMBER_OF_SLAVES 8

#define MAX_NUMBER_OF_CLIENTS 8

/* Server Functions */
// Create a socket
// Return the socket created or a error code ( -1: error )
int32_t create_socket();

// Binds the connection
// Requires the socket descriptor, the ip and the port
// Return a status value ( 0: error )
uint8_t bind_connection( int32_t* socket_descriptor, const char* ip, uint16_t port );

// Start listening on the socket
// Requires the socket descriptor and the uint32_t maximum number of connections to queue
// Returns a status value whether the starting of the listening was sucessfull ( 0: error )
uint8_t listen_connection( int32_t* socket_descriptor, uint32_t maximum_connections );

// Accept connections from the slave / clients
// Requires the socket descriptor and the function where to send the connection
// Handler function can have 1 argument
// In case of success return the id of the thread, else a error code ( 0: error )
pthread_t accept_connection( int32_t* socket_descriptor, void*( *handle_function )( void* ) );

// Can be called in case of errors
// Exits from the program
void connection_error();

/* Subprocess Functions */
// Subprocess to handle the slaves server
void slave_handler_subprocess();

// Subprocess to handle the clients server
void client_handler_subprocess();

int main()
{
	printf( "Starting...\n" );

	slave_handler_subprocess();

	return 0;
}

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

	// Accept the new connection
	new_socket_descriptor = accept( *socket_descriptor, ( struct sockaddr* )&address, &address_size );

	// Start the thread that handles the connection
	pthread_create( &thread_id, NULL, handle_function, &new_socket_descriptor );

	return 1;
}

void connection_error()
{
	perror( "An errros has occurred" ); // Print the occurred error
	exit( 0 );
}

void slave_handler_subprocess()
{
	int32_t socket_descriptor = 0;

	pthread_t thread = 0;

	uint8_t status_code = 0;

	// Create the socket
	socket_descriptor = create_socket();

	// Check that the creation of the socket was sucessfull
	if ( socket_descriptor == -1 )
		connection_error();

	status_code = bind_connection( &socket_descriptor, "127.0.0.1", 1234 );

	// Check that the binding was sucessfull
	if ( !status_code )
		connection_error();
	
	status_code = listen_connection( &socket_descriptor, MAX_NUMBER_OF_SLAVES );
	
	// Check that the start of listening was sucessfull
	if ( !status_code )
		connection_error();
	
	// Accept connection in loop of slaves
	while ( 1 )
	{
		thread = accept_connection( &socket_descriptor, handle_slave );

		// Check that the was sucessfull
		if ( !status_code )
			connection_error();
	}

	return;
}

void client_handler_subprocess()
{}