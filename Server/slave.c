#include "slave.h"

#define BUFFER_SIZE 1024

// Describes a slave
typedef struct
{
	int a;
} slave_t;

// Private Function Declarations

// 
uint32_t get_slave_id( char* slave_id );

// Get from the slave file the informations about a particular slave
// It is researched based on the slave's id ( parameter )
slave_t get_slave( uint32_t slave_id );

// Stores in the buffer the package to send
// The slave is given to select the correct package
void package_to_send( slave_t slave, uint8_t buffer );

uint8_t handle_slave( uint32_t socket_descriptor )
{
	// Alloc a Buffer to store informations
	uint8_t* buffer = ( uint8_t* )malloc( BUFFER_SIZE );

	// Clear the buffer
	// bzero( buffer, BUFFER_SIZE );

	// Recive the basic informations from the slave
	// Informations = "[number( 0 - 9 )]"
	recv( socket_descriptor, buffer, BUFFER_SIZE, 0 );

	uint32_t slave_id = get_slave_id( buffer );

	slave_t slave = get_slave( slave_id );

	// bzero( buffer );

	package_to_send( slave, buffer );

	// Free the buffer
	free( buffer );
}

uint32_t get_slave_id( char* slave_id )
{}

// Private Functions
slave_t get_slave( uint32_t slave_id )
{
	// Search a slave in the file
}

void package_to_send( slave_t slave, uint8_t buffer )
{}