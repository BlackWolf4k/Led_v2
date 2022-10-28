#include <stdio.h>
#include <stdint.h>


int main()
{
	FILE* file = NULL;

	file = fopen( "../Server/slaves.dat", "w" );

	uint32_t id = 0;

	printf( "Written: %d bytes\n" , fwrite( &id, sizeof( uint32_t ), 1, file ) );


	fclose( file );
}