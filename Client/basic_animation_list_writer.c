#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	FILE* file = NULL;

	file = fopen( "../Server/execution_space/animations_list.dat", "w" );

	uint8_t* buffer = ( uint8_t* )calloc( 1024, sizeof( uint8_t ) );

	buffer[0] = 0;

	strcpy( ( char* )( buffer + 1 ), ";debug.dat;\n" );

	printf( "Written: %ld\n" , fwrite( buffer, ( sizeof( uint8_t ) * strlen( ( char* )buffer + 1 ) ) + 1, 1, file ) );

	bzero( buffer, 1024 );

	buffer[0] = 1;
	strcpy( ( char* )( buffer + 1 ), ";jacket_rainbow.dat;\n" );

	printf( "Written: %ld\n" , fwrite( buffer, sizeof( uint8_t ) * strlen( ( char* )buffer ), 1, file ) );

	fclose( file );
}