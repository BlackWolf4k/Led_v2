#include "client.h"

#define BUFFER_SIZE 1024

#define CLIENT_REQUESTS 4
const char* client_requests[CLIENT_REQUESTS] = { "get_slaves", // asking for the list of slaves
								   "get_animations_names", // asking for a slaves animation playlist
								   "get_animation", // asking for animation data
								   "upload_animation" // uploading an animation
								   };

// Create a mutex to lock when file positioning is needed
pthread_mutex_t mutex_client;

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

/*Private Functions*/
// Get all the slaves associated with the server
// Requires as argument the socket desciptor of the client connection
// Returns a success or failure status ( 0: failure, 1: success )
uint8_t send_slaves( int32_t socket_descriptor );

// Send the animation names of a playlist
// Requires as argument the socket descriptor of the client connection
// Returns a success or failure status ( 0: failure, 1: success )
uint8_t send_animations_names( int32_t socket_descriptor );

// Send an animation
// Requires as argument the socket descriptor of the client connection
// Returns a success or failure status ( 0: failure, 1: success )
uint8_t send_animation( int32_t socket_descriptor );

// Recives a animation from the client
// Requires as argument the socket descriptor of the client connection
// Returns a success or failure status ( 0: failure, 1: success )
uint8_t download_animation( int32_t socket_descriptor );

// Add a animation to the animations file
// Requires as argument the filename and the animation list id
// Returns as argument the number of the animation added
uint8_t add_animation( char* filename, int32_t animation_list );

// Include function from "slave.c"
extern slave_t get_slave( uint32_t slave_id );

// Include function from "slave.c"
extern uint8_t update_slave( slave_t updated_slave );

uint8_t ( *handling_functions[CLIENT_REQUESTS] )( int32_t ) = { send_slaves,
															 send_animations_names,
															 send_animation,
															 download_animation
															};

void* handle_client( void* socket_descriptor )
{
	int32_t bytes_recived = 0;

	// Create a buffer to store informations
	char* buffer = NULL;
	buffer = calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Recive the request from the client
	bytes_recived = recv( *( int32_t* )socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 );

	if ( bytes_recived <= 0 )
	{
		exit( 2 );
	}

	uint8_t go = 1;

	// Decide what to do
	for ( uint32_t i = 0; i < CLIENT_REQUESTS && go; i++ )
		if ( strstr( buffer, client_requests[i] ) != NULL )
		{
			if ( !( ( handling_functions[i] )( *( int32_t* )socket_descriptor ) ) )
				exit( 2 );
			else
				go = 0;
		}
	
	printf( "Closing Connection\n" );

	// Close the socket
	close( *( int32_t* )socket_descriptor );
}

uint8_t send_slaves( int32_t socket_descriptor )
{
	printf( "Sending slaves to the client\n" );

	// Open the file
	FILE* file = NULL;
	file = fopen( "slaves.dat", "r+" );

	// Create a buffer to store informations
	char* buffer = NULL;
	buffer = calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the file opening and memory allocation were sucessfull
	if ( file == NULL || buffer == NULL )
	{
		perror( "[File Error]" );
		return 0; // Return the slave with error code id
	}

	uint32_t i = 0;

	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex_client );

	// Read each element in the file untill slave with corresponding id is found
	while ( fread( buffer + i, sizeof( slave_t ), 1, file ) != 0 )
	{
		printf( "Slave: %hhu\n", ( ( slave_t* )( buffer + i ) ) -> id );
		i += sizeof( slave_t );

		// Check that there is enought space in the buffer
		if ( i + sizeof( slave_t ) > BUFFER_SIZE )
		{
			// Send the slave
			// Check that the sending was sucessfull
			if ( send( socket_descriptor, buffer, BUFFER_SIZE, 0 ) <= 0 )
			{
				// There was an error while sending
				perror( "[Sending Error]" );
				return 0;
			}

			// Clear the buffer
			bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );
			i = 0;
		}
	}

	// Send the slaves
	// Check that the sending was sucessfull
	if ( send( socket_descriptor, buffer, BUFFER_SIZE, 0 ) <= 0 )
	{
		// There was an error while sending
		perror( "[Sending Error]" );
		return 0;
	}

	// Unlock
	pthread_mutex_unlock( &mutex_client );
	/* End of possible collision zone */

	// Free the buffer
	free( buffer );

	// Close the file
	fclose ( file );

	printf( "Slaves Sended\n" );

	return 1;
}

uint8_t send_animations_names( int32_t socket_descriptor )
{	
	printf( "Sending animation playlist to the Client\n" );

	// Create a buffer to store data
	uint8_t* buffer_base = NULL;
	uint8_t* buffer = buffer_base = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( buffer_base == NULL )
	{
		perror( "[Memory Error]" );
		return 0;
	}

	// Recive the slave
	if ( recv( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
	{
		perror( "[Reciving Error]" );

		// Free the buffer
		free( buffer );

		return 0;
	}

	// Get the slave
	slave_t slave = get_slave( buffer[0] );

	if ( slave.id == 0 )
	{
		printf( "Wrong slave id: %hhu\n", slave.id );

		// Send empty packet
		send( socket_descriptor, buffer + 1, 1, 0 );

		// Free the buffer
		free( buffer );

		return 0;
	}

	printf( "Recived slave id: %hhu\n", slave.id );

	// Clear the buffer
	bzero( buffer, sizeof( slave_t ) + 1 );

	// Open the file
	FILE* file = NULL;
	file = fopen( "animations_list.dat", "r" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		perror( "[File Error]" );
		return 0; // Return empty string
	}

	uint8_t found = 0;

	/* Send the animations names */
	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex_client );

	// Read each line of the file untill the slave id is found
	while ( fgets( buffer, BUFFER_SIZE, file ) && !found )
	{
		if ( slave.animation_list == buffer[0] )
		{
			if ( send( socket_descriptor, buffer + 2, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
			{
				perror( "[Sending Error]" );

				// Free the buffer
				free( buffer );

				//Close the file
				fclose( file );

				// Unlock
				pthread_mutex_unlock( &mutex_client );

				return 0;
			}
			found = 1;
		}
	}

	// Unlock
	pthread_mutex_unlock( &mutex_client );
	/* End of possible collision zone */

	// Close the file
	fclose( file );

	// Free the buffer
	free( buffer );

	printf( "Animation list sended\n" );

	return 1;
}

uint8_t send_animation( int32_t socket_descriptor )
{
	printf( "Sending animation to Client\n" );
	// Create a buffer to store informations
	char* buffer = NULL;
	buffer = calloc( BUFFER_SIZE, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( buffer == NULL )
	{
		// There was an error
		perror( "[Memory Allocation]" );
		return 0;
	}

	// Recive wich animation to send
	// Check that the reciving was sucessfull
	if ( recv( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
	{
		// Free the buffer
		free( buffer );

		// There was an error
		perror( "[Reciving Error]" );
		return 0;
	}

	// send_file( buffer, socket_descriptor );

	// Open the file
	FILE* file = NULL;
	file = fopen( buffer, "r" );

	// Clear the buffer
	bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );

	// Check that the file exists
	if ( file == NULL )
	{
		// Send a error message
		// Check that the sending was sucessfull
		if ( send( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
		{
			// Free the buffer
			free( buffer );

			// There was an error
			perror( "[Opening Error]" );
			return 0;
		}
	}

	/* Send the animation */
	/* Start of possible collision zone */
	// Lock
	pthread_mutex_lock( &mutex_client );

	// Read the animation file descriptor
	// Check that the reading was sucessfull
	if ( !fread( buffer, sizeof( animation_file_descriptor_t ), 1, file ) )
	{
		// There was an error
		// Close the file
		fclose( file );

		// Free the buffer
		free( buffer );

		// Unclock
		pthread_mutex_unlock( &mutex_client );

		return 0;
	}

	// Send the animation file descriptor
	// Check that the sending was sucessfull
	if ( send( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
	{
		// There was an error
		// Close the file
		fclose( file );

		// Free the buffer
		free( buffer );

		// Unclock
		pthread_mutex_unlock( &mutex_client );

		perror( "[Sending Error]" );
		return 0;
	}

	animation_file_descriptor_t animation_file_descriptor = *( ( animation_file_descriptor_t* )buffer );

	// Clear the buffer
	bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );

	for ( uint32_t i = 0; i < animation_file_descriptor.number_of_lines; i++ )
	{
		// Read the animation piece
		if ( !fread( buffer, animation_file_descriptor.line_length, 1, file ) )
		{
			// There was an error
			// Close the file
			fclose( file );

			// Free the buffer
			free( buffer );

			// Unclock
			pthread_mutex_unlock( &mutex_client );

			return 0;
		}

		// Send the animation piece
		// Check that the sending was sucessfull
		if ( send( socket_descriptor, buffer, animation_file_descriptor.line_length, 0 ) <= 0 )
		{
			// There was an error
			// Close the file
			fclose( file );

			// Free the buffer
			free( buffer );

			// Unclock
			pthread_mutex_unlock( &mutex_client );

			perror( "[Sending Error]" );
			return 0;
		}

		// Clear the buffer
		bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );
	}
	
	// Unlock
	pthread_mutex_unlock( &mutex_client );
	/* End of possible collision zone */

	// Close the buffer
	free( buffer );

	// Close the file
	fclose( file );

	printf( "Animation Sended\n" );

	// Everything was fine
	return 1;
}

uint8_t download_animation( int32_t socket_descriptor )
{
	printf( "Downloading an animation\n" );

	// Allocate a buffer
	uint8_t* buffer = NULL;
	uint8_t* filename = NULL;
	buffer = ( uint8_t* )calloc( BUFFER_SIZE, sizeof( uint8_t ) );
	filename = ( uint8_t* )calloc( 20, sizeof( uint8_t ) );

	// Check that the memory allocation was sucessfull
	if ( buffer == NULL || filename == NULL )
	{
		// There was an error
		perror( "[Memory Allocation]" );
		return 0;
	}

	// Recive the filename, the animation file descriptor and other two bytes: 0 = slave id, 1 = force next play
	// Check that the reciving was sucessfull
	if ( recv( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
	{
		// Free the buffer
		free( buffer );

		// There was an error
		perror( "[Reciving Error 0]" );
		return 0;
	}

	printf( "Filename: %s\n", buffer );
	for ( int i = 0; i < BUFFER_SIZE; i++ )
		printf( "%hhu-", buffer[i]);
	
	// Get the end of the filename
	// int32_t file_name_end = 0;

	// file_name_end = ( uint8_t* )( strstr( buffer, ".dat" ) ) - buffer;

	printf( "FileName length: %d\n\n", strlen( buffer ) );
	// Save the filename
	memcpy( filename, buffer, strlen( buffer ) ); // + 4 for len of ".dat"

	// Open the file
	FILE* file = NULL;
	file = fopen( filename, "wb+" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		// Free the buffer
		free( buffer );

		// There was an error
		perror( "[Opening Error]" );
		return 0;
	}

	// Write the descriptor
	// Check that the writing was sucessfull
	if ( !fwrite( buffer + strlen( buffer ) + 1, sizeof( animation_file_descriptor_t ), 1, file ) )
	{
		perror( "[Writing Error]" );

		// Free the buffer
		free( buffer );

		// Close the file
		fclose( file );

		return 0;
	}

	// Store the animation file descriptor
	animation_file_descriptor_t descriptor = *( animation_file_descriptor_t* )( buffer + strlen( buffer ) + 1 );

	uint8_t slave_id = *( buffer + ( strlen( buffer ) + sizeof( animation_file_descriptor_t ) ) );
	uint8_t forced = *( buffer + ( strlen( buffer ) + sizeof( animation_file_descriptor_t ) + 1 ) );

	printf( "Slave id: %hhu\n", slave_id );
	printf( "Number of Lines: %u\n", descriptor.number_of_lines );
	printf( "Line Length: %u\n", descriptor.line_length );
	printf( "Delay: %hhu\n", descriptor.delay );

	// Clear the buffer
	bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );

	// Write the animation body
	// Keep on going untill stops reciving
	for ( uint32_t i = 0; i < descriptor.number_of_lines; i++ )
	{
		// Check if reciving was sucessfull
		if ( recv( socket_descriptor, buffer, descriptor.line_length * sizeof( uint8_t ), 0 ) <= 0 )
		{
			perror( "[Reciving Error]" );

			// Free the buffer
			free( buffer );

			// Close the file
			fclose( file );

			return 0;
		}

		// Write each piece
		// Check that the writing was sucessfull
		if ( !fwrite( buffer, BUFFER_SIZE * sizeof( uint8_t ), 1, file ) )
		{
			perror( "[Writing Error]" );

			// Free the buffer
			free( buffer );

			// Close the file
			fclose( file );

			return 0;
		}

		// Clear the buffer
		bzero( buffer, BUFFER_SIZE * sizeof( uint8_t ) );
	}

	// Update the slave
	slave_t slave = get_slave( slave_id );

	if ( forced )
		slave.actual_animation = add_animation( filename, slave.animation_list );

	update_slave( slave );

	// Close the file
	fclose( file );

	// Free the buffer
	free( buffer );

	printf( "Animation downloaded\n" );
}

uint8_t add_animation( char* filename, int32_t animation_list )
{
	// Open the animation file
	FILE* file = NULL;
	file = fopen( "./animations_list.dat", "rb+" );

	// Check that the file opening was sucessfull
	if ( file == NULL )
	{
		// There was an error
		perror( "[Opening Error]" );
		return 0;
	}

	// Get the size of the file
	fseek( file, 0L, SEEK_END );
	uint32_t size = ftell( file );

	// Get to the beginning
	fseek( file, 0L, SEEK_SET );

	// Allocate a buffer of the length of the file
	uint8_t* buffer = NULL;
	buffer = ( uint8_t* )calloc( size, sizeof( uint8_t ) );

	// Check that the allocation was sucessfull
	if ( buffer == NULL )
	{
		// Close the file
		fclose( file );

		// There was an error
		perror( "[Memory Allocation]" );
		return 0;
	}

	// Store the file in the buffer
	if ( !fread( buffer, sizeof( uint8_t ), size, file ) )
	{
		// Close the file
		fclose( file );

		// Free the buffer
		free( buffer );

		// Print a error message
		perror( "[Reading Error]" );
		return 0;
	}

	// Get to the beginning
	fseek( file, 0L, SEEK_SET );

	uint8_t previous_character = '\n';
	uint8_t character = '\0';
	uint32_t number = 0;

	// Get the animation playlist
	// Read each line of the file untill the slave id is found
	while ( fread( &character, sizeof( uint8_t ), 1, file ) )
	{
		if ( character == animation_list && previous_character == '\n' )
		{
			// Save the position
			uint32_t position = 0;

			// Read untill the place where to put the animation is found
			while ( fread( &character, sizeof( uint8_t ), 1, file ) )
			{
				if ( character == '\n' )
				{
					fseek( file, -1L, SEEK_CUR );
					// Get current position
					position = ftell( file );
					printf( "Position: %d", position);
					fwrite( filename, sizeof( uint8_t ), strlen( filename ), file );
					fwrite( ";", sizeof( uint8_t ), 1, file );
					break;
				}
				else if ( character == ';' )
					number += 1;
			}

			if ( fwrite( buffer + position, sizeof( uint8_t ), size - position, file ) )
			{
				// Free the buffer
				free( buffer );

				// Close the file
				fclose( file );

				// Exit
				return number;
			}
		}
		else
		{
			previous_character = character;
		}
	}

	// Free the buffer
	free( buffer );

	// Close the file
	fclose( file );

	return 0;
}