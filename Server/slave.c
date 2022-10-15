#include "slave.h"

#define BUFFER_SIZE 1024

// Create a mutex to lock when file positioning is needed
pthread_mutex_t mutex;

/*
SLAVE CONNECTION DESCRIPTOR
id: 			the id of the slave
ip_address: 	the ip address of the slave
*/
typedef struct
{
	uint8_t id;
	char ip_address[16];
} slave_connection_t;

/*
SLAVES DESCRIPTOR:
id:					identifier of a slave
actual_animation:	the currently plaing animation
status:				if waiting for something, or plaing and animation
animation_list:		the id of the animation list
ip_address:			ip address of the slave
*/
typedef struct
{
	uint8_t id;
	uint8_t status;
	uint8_t animation_list;
	uint8_t actual_animation;
	char ip_address[16];
} slave_t;

/*
ANIMATION LIST DESCRIPTION:
list_id;file_name;file_name... ( 50 animations list MAX )
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
repeat:				does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
*/
typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t repeat;
} animation_file_descriptor_t;

// Animation header size ( +1 for the \n )
#define ANIMATION_HEADER_SIZE ( sizeof( animation_file_descriptor_t ) + 1 )

/*PRIVATE FUNCTIONS DECLARATION*/

// Get from the slave file the informations about a particular slave
// It is researched based on the slave's id ( parameter ) and the slave's ip address
// If no slave was found it will return a slave with id -1
// The id of a slave does not chagne, while the ip can, so the ip may be updated
slave_t get_slave( uint32_t slave_id, char* ip_address );

// Get the next animation of slave
// The animation list number and the actual animation are needed
// The file name of the next animation will be returned
// If and error occured, an empry string is returned
const char* get_next_animation( uint32_t animation_list_id, uint32_t animation_number );

// Send the animation file to the slave
// Returns a status code for success of failure
uint8_t send_file( const char* animation_file, uint32_t slave_socket );

uint8_t* handle_slave( uint32_t socket_descriptor )
{
	// Create a buffer to store informations
	uint8_t* buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Recive the basic informations from the slave
	recv( socket_descriptor, buffer, BUFFER_SIZE, 0 );

	slave_connection_t slave_connection = *( ( slave_connection_t* )buffer );

	// Send to the slave a ack message that everything was recived
	buffer[0] = 1;
	send( socket_descriptor, buffer, 1, 0 );

	// Get the slave descriptor
	slave_t slave = get_slave( slave_connection.id, slave_connection.ip_address );

	// Check that the slave research was sucessfull

	// Get next animation
	const char* file_name = get_next_animation( slave.animation_list, slave.actual_animation + 1 );

	// Check that the next animation was found

	// Send the animation
	send_file( file_name, socket_descriptor );

	// Check that the sending was sucessfull

	/* Update the slave informations in the file */
	// Close the connection
	close( socket_descriptor );

	// Open the file
	FILE* file = NULL;
	fopen( "slaves.dat", "rw" );

	// Check that the file opening was sucessfull
	// if ( file == NULL )
	// 	return 0;

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	// Unlock
	pthread_mutex_unlock( &mutex );
	/* End of possible collision zone */

	// Close the file
	fclose( file );
}

/*PRIVATE FUNCTIONS*/

slave_t get_slave( uint32_t slave_id, char* ip_address )
{
	// Slave to return in case of errors
	slave_t slave_error;
	// Give to the slave the id of -1, it means error
	slave_error.id = -1;

	// Open the file
	FILE* file = NULL;
	file = fopen( "slaves.dat", "rw" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		// Close the file
		fclose( file );
		return slave_error; // Return the slave with error code id
	}
	
	// To store the readed slave
	slave_t slave;

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	// Read each element in the file untill slave with corresponding ip is found
	while ( fread( &slave, sizeof( slave_t ), 1, file ) != 0 )
	{
		// Check that the slave id is the same of the one read
		if ( slave.id == slave_id )
		{
			// Check if the slave informations need to be updated
			if ( strcmp( slave.ip_address, ip_address ) != 0 )
			{
				// Update the ip
				strcpy( slave.ip_address, ip_address );

				// Update the slave on the file
				fseek( file, -1 * sizeof( slave_t ), SEEK_CUR );
				fwrite( &slave, sizeof( slave_t ), 1, file );
			}
			// Unlock
			pthread_mutex_unlock( &mutex );

			// Close the file
			fclose( file );

			return slave;
		}
	}

	// No slave with the corresponding id was found
	// Add it

	// Unlock
	pthread_mutex_unlock( &mutex );
	/* End of possible collision zone */

	// Close the file
	fclose( file );

	// No slave was found, so return the slave with error code id
	return slave_error;
}

const char* get_next_animation( uint32_t animation_list_id, uint32_t animation_number )
{
	// Open the file
	FILE* file = NULL;
	file = fopen( "animation_list", "r" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		// Close the file
		fclose( file );
		return '\0'; // Return empty string
	}
	
	// Create a buffer to store data
	uint8_t* buffer_base = NULL;
	uint8_t* buffer = buffer_base = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	// Store the file name to return
	char* file_name = NULL;

	// Read each line of the file untill the slave id is found
	while ( fgets( buffer, BUFFER_SIZE, file ) )
	{
		if ( ( ( uint32_t* )buffer )[0] == animation_list_id )
		{
			// Count that animation number is valid
			uint8_t animations = 0;

			for ( uint32_t i = 0; i < BUFFER_SIZE; i++ )
				if ( buffer[i] == ';' )
					animations += 1;
			
			// Check that there are enought animation
			if ( animations - 1 < animation_number )
			{
				// Close the file
				fclose( file );
				// Unlock
				pthread_mutex_unlock( &mutex );
				return '\0'; // Return empty string
			}

			buffer += 5; // 4 of id, 1 for ';'

			// Search the animation
			for ( int i = 0; i < animation_number; i++ )
			{
				file_name = strtok( buffer, ";" );
				buffer += strlen( file_name ) + 1;
			}
		}
	}

	// Unlock
	pthread_mutex_unlock( &mutex );
	/* End of possible collision zone */

	// Free the buffer
	free( buffer_base );
	// Close the file
	fclose( file );

	return file_name;
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