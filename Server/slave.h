#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>

// Function called by the server
// Return a status code for success or faiuler
// Requires the slaved id ( sent on connection by the slave ) as parameter
uint8_t handle_slave( uint32_t socket_descriptor );