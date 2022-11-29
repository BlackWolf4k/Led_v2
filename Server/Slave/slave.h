#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Function called by the server
// Requires the socket descriptor of the connection with the slave
// Return nothing
void* handle_slave( void* socket_descriptor );

// Function ordered by a client
// Sent to a slave an animation, forcing it to stop everything he was doing
// Returns a status code for success or failure
// Requires
uint8_t force_slave();