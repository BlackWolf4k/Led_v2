#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

// Function called by the server
// Return a status code for success or faiuler
// Requires the slaved id ( sent on connection by the slave ) as parameter
uint8_t handle_slave( uint32_t socket_descriptor );

// Function ordered by a client
// Sent to a slave an animation, forcing it to stop everything he was doing
// Returns a status code for success or failure
// Requires
uint8_t force_slave();