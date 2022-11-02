#include <stdio.h>

#include "pico/stdlib.h"
// Wireless driver
#include "pico/cyw43_arch.h"
// For tcp comunication
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include <string.h>

typedef struct
{
	struct tcp_pcb* tcp_pcb;
	ip_addr_t server_address;
	uint8_t* buffer;
} client_t;

#define BUFFER_SIZE 1024

// Initialize a new client
// Requires no argument
// Returns a pointer to the new client if sucessfull, else returns a NULL pointer
client_t* client_init();

// Open a connection from a client to a host
// Requires as argument the client, the ip of the host to connect to and the port
// Return a status code where the opening was sucessfull ( 0: error )
uint8_t connect( client_t* client, const char* ip_address, uint16_t port );

// Closes the connection of a client to a host
// Requires the client as argument
// Return a status code where the closing was sucessfull ( 0: error )
uint8_t close( client_t* client );

// Send data
// Requires
// Returns
uint8_t send( client_t* client );

// Recive data
// Requires
// Returns
uint8_t recv();