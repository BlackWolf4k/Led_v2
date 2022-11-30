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

	// Decide what to do
	for ( uint32_t i = 0; i < CLIENT_REQUESTS; i++ )
		if ( strstr( buffer, client_requests[i] ) != NULL )
			if ( !( ( handling_functions[i] )( *( int32_t* )socket_descriptor ) ) )
				exit( 2 );
	
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
	printf( "Sending animation playlist to the Client" );

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
	if ( recv( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t ), 0 ) )
	{
		perror( "[Reciving Error]" );

		// Free the buffer
		free( buffer );

		return 0;
	}

	// Get the slave
	slave_t slave = *( ( slave_t* )( buffer ) );

	// Clear the buffer
	bzero( buffer, sizeof( slave_t) + 1 );

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
			if ( send( socket_descriptor, buffer + 1, BUFFER_SIZE * sizeof( uint8_t ), 0 ) <= 0 )
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
	if ( recv( socket_descriptor, buffer, BUFFER_SIZE * sizeof( uint8_t), 0 ) <= 0 )
	{
		// Free the buffer
		free( buffer );

		// There was an error
		perror( "[Reciving Error]" );
		return 0;
	}

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
			// Close the file
			fclose( file );

			// Free the buffer
			free( buffer );

			// There was an error
			perror( "[Sending Error]" );
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
	// Recive the name
	// Recive the animation
	printf( "Animation downloaded\n" );
}
