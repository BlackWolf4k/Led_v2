#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
// Socket
#include <sys/socket.h>
#include <arpa/inet.h>

#include "./slave.h"

#define MAX_NUMBER_OF_SLAVES 8

// The slaves connect on port 1234

// Handle the slaves
void handle_slaves_subprocess();
// Function called if any part of slave's server subprocess starting failed
void slave_server_error();
// Handle the client
void handle_clients();

int main()
{
	handle_slaves_subprocess();

	return 0;
}

void handle_slaves_subprocess()
{
	// Create the socket
	uint32_t socket_descriptor = socket( AF_INET, SOCK_STREAM, 0 );

	// Check that the socket creation was sucessfull
	if ( socket_descriptor < 0 )
		slave_server_error();

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	server_address.sin_port = 1234;

	// Bind the connection
	uint32_t established = bind( socket_descriptor, ( struct sockaddr* )&server_address, sizeof( struct sockaddr* ) );

	// Check if the binding was sucessfull
	if ( established < 0 )
		slave_server_error();

	established = listen( socket_descriptor, MAX_NUMBER_OF_SLAVES );

	// Check if listening start was sucessfull
	if ( established < 0 )
		slave_server_error();

	uint32_t new_slave_descriptor = 0;
	struct sockaddr_in address;
	socklen_t address_size = sizeof( address );
	
	// Accept the connections
	while ( 1 )
	{
		// pthread_t* thread_id = ( pthread_t* )malloc( MAX_NUMBER_OF_SLAVES * sizeof( pthread_t ) );
		pthread_t thread_id = 0;

		new_slave_descriptor = 0;

		new_slave_descriptor = accept( socket_descriptor, ( struct sockaddr* )&address, &address_size );

		pthread_create( &thread_id, NULL, handle_slave, &new_slave_descriptor );
	}
}

void slave_server_error()
{
	perror( "An errros has occurred" ); // Print the occurred error
	exit( 0 );
}