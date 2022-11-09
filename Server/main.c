#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// For threads
#include <pthread.h>
// For Fork
#include <unistd.h>
#include <sys/wait.h>
// For Server side functions
#include "./Server/server.h"
// For slaves handling
#include "./Slave/slave.h"

#define MAX_NUMBER_OF_SLAVES 8
#define MAX_NUMBER_OF_CLIENTS 8

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

void slave_handler_subprocess()
{
	int32_t socket_descriptor = 0;

	pthread_t thread = 0;

	uint8_t status_code = 0;

	printf( "Creating Socket for Slaves\n" );

	// Create the socket
	socket_descriptor = create_socket();

	// Check that the creation of the socket was sucessfull
	if ( socket_descriptor == -1 )
		connection_error();
	
	printf( "Binding Connection for Slaves\n" );

	status_code = bind_connection( &socket_descriptor, "192.168.0.1", 1234 );

	// Check that the binding was sucessfull
	if ( !status_code )
		connection_error();
	
	printf( "Starting to listen for Slaves\n" );
	
	status_code = listen_connection( &socket_descriptor, MAX_NUMBER_OF_SLAVES );
	
	// Check that the start of listening was sucessfull
	if ( !status_code )
		connection_error();
	
	printf( "Starting to accept connections from Slaves\n" );
	
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