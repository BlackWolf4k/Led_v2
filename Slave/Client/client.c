#include "client.h"

/*Private Functions*/

// Handle a error in the connection
// Requires
// Returns
uint8_t connection_error();

client_t* client_init()
{
	// Allocate the client
	client_t* client = NULL;
	client_t* client = calloc( 1, sizeof( client_t ) );

	// Allocate a space for the buffer
	client -> buffer = calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( client == NULL || client -> buffer == NULL )
	{
		perror( "[Memory Error]" );
		return NULL; // Returns a null pointer
	}

	// Return the client
	return client;
}

uint8_t connect( client_t* client, const char* ip_address, uint16_t port );
{
	client -> server_pcb = tcp_new_ip_type( IP_GET_TYPE( &( client -> remote_addr ) ) );

	tcp_arg( client -> server_pcb, client );
	tcp_sent( client -> server_pcb, );
	tcp_recv( client -> server_pcb, );
	// tcp_poll( client -> server_pcb, );
	// tcp_err( client -> server_pcb, );

	cyw43_arch_lwip_begin();

    int32_t status_code = tcp_connect( client -> server_pcb, &( client -> remote_addr ), port, NULL );

    cyw43_arch_lwip_end();
}

uint8_t close( client_t* client )
{
	tcp_arg( client -> server_pcb, NULL );
	tcp_sent( client -> server_pcb, NULL );
	tcp_recv( client -> server_pcb, NULL );
	// tcp_poll( client -> server_pcb, NULL, 0 );
	// tcp_err( client -> server_pcb, NULL );

	tcp_close( client -> server_pcb );
}

uint8_t send( client_t* client )
{
	cyw43_arch_lwip_check();
	strcpy( client -> buffer, "1;255.252.111.222" );
	tcp_write( client -> server_pcb, client -> buffer, BUFFER_SIZE, TCP_WRITE_FLAG_COPY );
}

uint8_t recv();