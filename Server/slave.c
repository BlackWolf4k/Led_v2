#include "slave.h"

#define BUFFER_SIZE 1024

// File Possible Length:
// ( 8b * 3 ) * 160 + ( 8b * 160 ) * n

// Create a mutex to lock when file positioning is needed
pthread_mutex_t mutex;

/*
SLAVES DESCRIPTOR:
id:					identifier of a slave
actual_animation:	the currently plaing animation
status:				if waiting for something, or plaing and animation
animation_list:		the id of the animation list
*/
typedef struct
{
	uint8_t id;
	uint8_t status;
	uint8_t animation_list;
	uint8_t actual_animation;
} slave_t;

/*
ANIMATION FILE DESCRIPTION:
animation_file_descriptor
led_informations
delay_infomration
...
*/

/*
ANIMATION FILE DESCRIPTOR
number_of_line:		the number of lines in the file
line_length:		the length of a single line
repeat:				does the animation repeat
*/

typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t repeat;
} animation_file_descriptor_t;

// Animation header size ( +1 for the \n )
#define ANIMATION_HEADER_SIZE ( sizeof( animation_file_descriptor_t ) + 1 )

// Private Function Declarations

// Set buffer valuse to 0;
void bzero( void* buffer, uint32_t size )
{
	for ( uint32_t i = 0; i < size; i++ )
		( ( uint8_t* )buffer )[i] = ( uint8_t )0;
}

// 
uint32_t get_slave_id( char* slave_id );

// Get from the slave file the informations about a particular slave
// It is researched based on the slave's id ( parameter )
slave_t get_slave( uint32_t slave_id );

// Send the animation file to the slave
// Returns a status code for success of failure
uint8_t send_file( const char* animation_file, uint32_t slave_socket );

uint8_t handle_slave( uint32_t socket_descriptor )
{
	// Alloc a Buffer to store informations
	uint8_t* buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Recive the basic informations from the slave
	// Informations = "[number( 0 - 9 )]"
	recv( socket_descriptor, buffer, BUFFER_SIZE, 0 );

	// Alloc array to store ip address
	char* ip_address = ( char* )calloc( 16, sizeof( uint8_t ) );

	// Copy the ip address of the slave
	for ( int i = 0; i < 16 && buffer[i] != ';'; i++ )
		ip_address[i] == buffer[i];

	uint32_t slave_id = get_slave_id( buffer );

	slave_t slave = get_slave( slave_id );

	bzero( buffer, BUFFER_SIZE );

	// Free the buffers
	free( ip_address );
	free( buffer );
}

uint32_t get_slave_id( char* slave_id )
{}

// Private Functions
slave_t get_slave( uint32_t slave_id )
{
	// Search a slave in the file
}

uint8_t send_file( const char* animation_file, uint32_t slave_socket )
{
	// Create and open a file
	FILE* file = NULL;
	/* A Animation file can't be opened by more than one thread at the time */
	file = fopen( animation_file, "r" );

	// Alloc a Buffer to store informations
	uint8_t* buffer = ( uint8_t* )calloc( ANIMATION_HEADER_SIZE, sizeof( uint8_t ) );

	// Check that the file was opened
	if ( file == NULL || buffer == NULL )
		return 1; // Error while opening the file

	// Read the header
	if ( fgets( buffer, ANIMATION_HEADER_SIZE, file ) == NULL )
		return 1;
	
	// Copy the header in a variable
	animation_file_descriptor_t animation_file_descriptor = *( ( animation_file_descriptor_t* )( buffer ) );
	
	// Send the animation file descriptor
	if ( send( slave_socket, buffer, BUFFER_SIZE, 0 ) == -1 )
		return 1;

	// Resize the buffer
	free( buffer );
	buffer = ( uint8_t* )calloc( animation_file_descriptor.line_length + 1, sizeof( uint8_t ) );

	// Store the bytes sent
	uint32_t bytes_sent = 0;

	// Read the line and check that it exitst
	while ( fgets( buffer, animation_file_descriptor.line_length + 1, file ) != NULL )
	{
		// Make sure to sent the whole line
		while ( animation_file_descriptor.line_length - bytes_sent > 0 )
		{
			// Send the buffer, and store the number of bytes sent
			bytes_sent += send( slave_socket, ( buffer + bytes_sent ), BUFFER_SIZE, 0 );

			// Check that there where no sending errors
			if ( bytes_sent == -1 )
				return 1; // Something went wrong
		}

		bytes_sent = 0;
		// Clear the buffer
		bzero( buffer, animation_file_descriptor.number_of_lines + 1 );
	}
	
	// Free the buffer
	free( buffer );
	// Close the file
	fclose( file );

	return 0;
}