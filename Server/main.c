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
// For clients handling
#include "./Client/client.h"

#define MAX_NUMBER_OF_SLAVES 8
#define MAX_NUMBER_OF_CLIENTS 8

#define NUMBER_OF_SERVERS 2

/*Functions to run slaves and clients servers*/
// Start the slaves server
void slave_server();

// Start the client server
void client_server();

/* Subprocess Functions */
/*
SUBPROCESS INFORMATIONS:
slave server has a exit code of 1
client server has a exit code of 2
*/
// Run a subprocess
// Requires has argument the handler function
// Returns the pid to the subprocess
int32_t start_subprocess( void ( *handler )() );

// A deamon to check that everything is running sucessfully
// I a subprocess fails it will run it back
// Requires a list of the pids
void deamon( int32_t* server_pids );

int main()
{
	printf( "Starting...\n" );

	// Pids of the servers
	// [0] = slave server, [1] = client server
	int32_t server_pids[NUMBER_OF_SERVERS];

	printf( "Starting slave server\n" );
	server_pids[0] = start_subprocess( slave_server );

	printf( "Starting client server\n" );
	server_pids[1] = start_subprocess( client_server );

	deamon( server_pids );

	return 0;
}

int32_t start_subprocess( void ( *handler )() )
{
	// Store the pid
	int32_t pid = 0;

	// Fork the process
	pid = fork();

	// Check who is running this part
	if ( pid == 0 ) // Child
		( *handler )(); // Run the server
	else // Father
		return pid; // Return the child pid
}

void deamon( int32_t* server_pids )
{
	// Process exit code
	int32_t exit_code = 0;

	// Wait any process forever
	while ( 1 )
	{
		// Wait a child process
		waitpid( -1, &exit_code, 0 );

		// Check wich process returned
		if ( exit_code == 1 ) // Slave server returned
			server_pids[0] = start_subprocess( slave_server ); // Run it back and store the pid
		else if ( exit_code == 2 ) // Client server returned
			server_pids[1] = start_subprocess( client_server ); // Run it back and store the pid
	}

	return;
}

void slave_server()
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

void client_server()
{
		int32_t socket_descriptor = 0;

	pthread_t thread = 0;

	uint8_t status_code = 0;

	printf( "Creating Socket for Client\n" );

	// Create the socket
	socket_descriptor = create_socket();

	// Check that the creation of the socket was sucessfull
	if ( socket_descriptor == -1 )
		connection_error();
	
	printf( "Binding Connection for Clients\n" );

	status_code = bind_connection( &socket_descriptor, "192.136.60.133", 1235 ); // change

	// Check that the binding was sucessfull
	if ( !status_code )
		connection_error();
	
	printf( "Starting to listen for Clients\n" );
	
	status_code = listen_connection( &socket_descriptor, MAX_NUMBER_OF_CLIENTS );
	
	// Check that the start of listening was sucessfull
	if ( !status_code )
		connection_error();
	
	printf( "Starting to accept connections from Clients\n" );
	
	// Accept connection in loop of clients
	while ( 1 )
	{
		thread = accept_connection( &socket_descriptor, handle_client );

		// Check that the was sucessfull
		if ( !status_code )
			connection_error();
	}

	return;
}