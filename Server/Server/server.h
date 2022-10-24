#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
// Types definitions
#include <stddef.h>

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