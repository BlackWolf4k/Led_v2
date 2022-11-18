#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

int main()
{
	FILE* file = NULL;

	file = fopen( "../Server/execution_space/slaves.dat", "w" );

	slave_t slave;

	bzero( slave.ip_address, 16 );

	slave.id = 1;
	slave.status = 0;
	slave.animation_list = 0;
	slave.actual_animation = 0;

	printf( "Written: %ld\n" , fwrite( &slave, sizeof( slave_t ), 1, file ) );

	slave.id = 2;
	slave.animation_list = 1;

	printf( "Written: %ld\n" , fwrite( &slave, sizeof( slave_t ), 1, file ) );

	fclose( file );
}