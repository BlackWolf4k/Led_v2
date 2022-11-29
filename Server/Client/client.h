#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h> // mutex
#include <unistd.h>

// Function called by the server
//
void* handle_client( void* socket_descriptor );