#include "slave.h"

#define BUFFER_SIZE 1024
#define ANIMATION_NAME_LENGTH 20

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
id: 				identifier of a slave
ip_address: 		ip address of the slave
status: 			if waiting for something, or plaing an animation
animation_list: 	the id of the animation list
actual_animation:	the currently plaing animation
*/
typedef struct
{
	uint8_t id;
	char ip_address[16];
	uint8_t status;
	uint8_t animation_list;
	uint8_t actual_animation;
} slave_t;

/*
ANIMATION LIST DESCRIPTION:
list_id;file_name;file_name... ( 50 animations list MAX )
ANIMATION FILE DESCRIPTION:
animation_file_descriptor
led_informations
...
*/

/*
ANIMATION FILE DESCRIPTOR
number_of_line: 	the number of lines in the file
line_length:		the length of a single line
delay:				the delay between each change of led colors
repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
pattern:			animations with a repeating patter ( 0: none, 1: rainbow )
*/
typedef struct
{
	uint32_t number_of_lines;
	uint32_t line_length;
	uint8_t delay;
	uint8_t repeat;
	uint8_t pattern;
} animation_file_descriptor_t;

#define ANIMATION_HEADER_SIZE ( sizeof( animation_file_descriptor_t ) )

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
char* get_next_animation( uint32_t animation_list_id, uint32_t animation_number );

// Send the animation file to the slave
// Returns a status code for success of failure
uint8_t send_file( const char* animation_file, int32_t slave_socket );

void* handle_slave( void* socket_descriptor )
{
	printf( "Started comunication with slave\n" );

	int32_t bytes_recived = 0;

	// Create a buffer to store informations
	uint8_t* buffer = NULL;
	buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	if ( buffer == NULL )
	{
		perror( "[Malloc Error]" );
		exit( 1 );
	}

	// Recive the basic informations from the slave
	bytes_recived = recv( *( int32_t* )socket_descriptor, buffer, BUFFER_SIZE, 0 );
	
	if ( bytes_recived <= 0 )
	{
		perror( "[Socket Error]" );
		exit( 1 );
	}

	slave_connection_t slave_connection = *( ( slave_connection_t* )buffer );

	printf( "Recived basic infromations from slave: %u\n", ( uint32_t )( slave_connection.id ) );

	// Get the slave descriptor
	slave_t slave = get_slave( slave_connection.id, slave_connection.ip_address );

	// Check that the slave research was sucessfull
	if ( slave.id == 0 )
	{
		printf( "Invalid slave id" );
		exit( 1 );
	}

	printf( "Connection from slave: %hhu, ip: %s, animation playlist: %hhu\n", slave.id, slave.ip_address, slave.animation_list );

	printf( "Searching for the next animation\n" );

	// Get next animation
	char* file_name = get_next_animation( slave.animation_list, slave.actual_animation );

	// Check that the next animation was found
	if ( file_name == "" )
	{
		printf( "Invalid animation name" );
		exit( 1 );
	}

	printf( "Sending the next animation to the slave\n" );

	// Send the animation
	// Check that the sending was sucessfull
	if ( send_file( file_name, *( int32_t* )socket_descriptor ) )
	{
		printf( "Error while sending the file to the slave" );
		exit( 1 );
	}

	printf( "Closing connection with slave\n" );

	/* Update the slave informations in the file */
	// Close the connection
	close( *( int32_t* )socket_descriptor );

	// Open the file
	FILE* file = NULL;
	file = fopen( "slaves.dat", "r+" );

	// Check that the file opening was sucessfull
	// if ( file == NULL )
	// 	return 0;

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	printf( "Updating information about the slave\n" );

	// Unlock
	pthread_mutex_unlock( &mutex );
	/* End of possible collision zone */

	// Close the file
	fclose( file );

	// Free the socket
	// Shouldn' t be here
	// I use arch btw
	free( socket_descriptor );

	// Free the file_name space
	free( file_name );

	printf( "Slave sucessfully served\n" );
}

/*PRIVATE FUNCTIONS*/

uint8_t recive_slave_descriptor();

slave_t get_slave( uint32_t slave_id, char* ip_address )
{
	// Slave to return in case of errors
	slave_t slave_error;
	// Give to the slave the id of 0, it means error
	slave_error.id = 0;

	// Open the file
	FILE* file = NULL;
	file = fopen( "slaves.dat", "r+" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		perror( "[File Error]" );
		return slave_error; // Return the slave with error code id
	}
	
	// To store the readed slave
	slave_t slave;

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	// Read each element in the file untill slave with corresponding id is found
	while ( fread( &slave, sizeof( slave_t ), 1, file ) != 0 )
	{
		// Check that the slave id is the same of the one read
		if ( slave.id == slave_id )
		{
			// Check if the slave informations need to be updated
			if ( strcmp( slave.ip_address, ip_address ) != 0 ) // No the same ip
			{
				// Update the ip
				strcpy( slave.ip_address, ip_address );

				// Update the slave on the file
				fseek( file, -1 * sizeof( slave_t ), SEEK_CUR );

				// Write the updated slave
				// Check that the writing was sucessfull
				if ( fwrite( &slave, sizeof( slave_t ), 1, file ) != 1 )
					printf( "Something went wrong while updating the slaves informations\n" );
			}
			// Unlock
			pthread_mutex_unlock( &mutex );

			// Close the file
			fclose( file );

			return slave;
		}
	}

	/*
	// No slave with the corresponding id was found
	// Add it
	// Copy the informations about the slave
	slave_t new_slave;
	new_slave.id = slave_id;
	strcpy( new_slave.ip_address, ip_address );

	// Write the slave
	if ( fwrite( &new_slave, sizeof( slave_t ), 1, file ) != 1 )
		printf( "Something went wrong while writing the slave\n" );
	*/

	// Unlock
	pthread_mutex_unlock( &mutex );
	/* End of possible collision zone */

	// Close the file
	fclose( file );

	// No slave was found, so return the slave with error code id
	return slave_error;
}

char* get_next_animation( uint32_t animation_list_id, uint32_t animation_number )
{
	// Open the file
	FILE* file = NULL;
	file = fopen( "animations_list.dat", "r" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		perror( "[File Error]" );
		return '\0'; // Return empty string
	}
	
	// Create a buffer to store data
	uint8_t* buffer_base = NULL;
	uint8_t* buffer = buffer_base = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( buffer_base == NULL )
	{
		perror( "[Memory Error]" );
		return '\0';
	}

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex );

	// Store the file name to return
	char* file_name = NULL;
	file_name = ( uint8_t* )calloc( ANIMATION_NAME_LENGTH, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( file_name == NULL )
	{
		perror( "[Memory Error]" );
		return '\0';
	}

	printf( "Looking for animation number: %d\n", animation_number );

	// Read each line of the file untill the slave id is found
	while ( fgets( buffer, BUFFER_SIZE, file ) )
	{
		if ( ( ( uint8_t* )buffer )[0] == animation_list_id )
		{
			// Animations number counter
			uint8_t animations = 0;

			// Counter the number of animations
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

			// Clear out the list id
			buffer += 2; // 1 of id, 1 for ';'

			// Search the animation
			for ( int i = 0; i <= animation_number; i++ )
			{
				strcpy( file_name, strtok( buffer, ";" ) );
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

uint8_t send_file( const char* animation_file, int32_t slave_socket )
{
	// Create and open a file
	FILE* file = NULL;
	/* A Animation file can't be opened by more than one thread at the time */
	file = fopen( animation_file, "r" );

	printf( "FileName: %s\n", animation_file );

	// Check that the file was opened
	if ( file == NULL )
	{
		perror( "[File Error]" );
		return 1; // Error while opening the file
	}

	// Alloc a Buffer to store informations
	uint8_t* buffer = NULL;
	buffer = ( uint8_t* )calloc( ANIMATION_HEADER_SIZE, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( buffer == NULL )
	{
		perror( "[Memory Error]" );
		return 1; // Error while allocating the memory
	}
	
	int32_t bytes_sent = 0;

	// Read the header
	if ( fread( buffer, ANIMATION_HEADER_SIZE, 1, file ) == 0 ) // +1 for \n
		return 1;
	
	// Copy the header in a variable
	animation_file_descriptor_t animation_file_descriptor = *( ( animation_file_descriptor_t* )( buffer ) );
	
	// Send the animation file descriptor
	bytes_sent = send( slave_socket, buffer, BUFFER_SIZE, 0 );
	perror( "[Socket Error]" );

	if ( bytes_sent <= 0 )
		return 1;

	// Resize the buffer
	free( buffer );
	buffer = ( uint8_t* )calloc( animation_file_descriptor.line_length, sizeof( uint8_t ) );

	// Store the bytes sent
	bytes_sent = 0;

	// Read the line and check that it exitst
	while ( fgets( buffer, animation_file_descriptor.line_length, file ) != NULL )
	{
		// Make sure to sent the whole line
		while ( ( int32_t )( animation_file_descriptor.line_length - bytes_sent ) > 0 )
		{
			// Send the buffer, and store the number of bytes sent
			bytes_sent += send( slave_socket, ( buffer + bytes_sent ), BUFFER_SIZE, 0 );

			// Check that there where no sending errors
			if ( bytes_sent == -1 )
				return 1; // Something went wrong
		}

		bytes_sent = 0;
		// Clear the buffer
		bzero( buffer, animation_file_descriptor.number_of_lines );
	}
	
	// Free the buffer
	free( buffer );
	// Close the file
	fclose( file );

	return 0;
}