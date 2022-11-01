#include <stdio.h>

#include "pico/stdlib.h"
// Wireless driver
#include "pico/cyw43_arch.h"
// For tcp comunication
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// Start the connection with the main server
// Send basic informations about the slave
// Requires the slave informations and the socket where to send data
// Return a status code where the sending was sucessfull ( 0: error )
uint8_t send_slave_connection_informations( int32_t socket_descriptor, slave_connection_t slave_informations );

// Recive the animation descriptor from the main server
// Requires a argument where to store the animation descriptor and the socket where to send data
// Return a status code where the reciving was sucessfull ( 0: error )
uint8_t recive_animation_descriptor( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Recive the animation colors and delays from the main server
// Requires the animation descriptor as argument and the socket where to send data
// Return a status code where the reciving was sucessfull ( 0: error )
uint8_t recive_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Play the animation
// Requires as argument the animation descriptor and the socket where to sand data
// Returns a status code ( 0: animation ended, 1: main server forced to quit, 0: error )
uint8_t play_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

// Check if the main server calls a forced stop
// The value at 0x is set to 1 for forced stop, else 0
// Requires no argument
// Returns nothing
void callback_server();

int main()
{
	stdio_init_all();
	cyw43_arch_init();
	cyw43_arch_wifi_connect_timeout_ms( WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000 );
	cyw43_arch_deinit();
}

uint8_t send_slave_connection_informations( int32_t socket_descriptor, slave_connection_t slave_informations );

uint8_t recive_animation_descriptor( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

uint8_t recive_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

uint8_t play_animation( int32_t socket_descriptor, animation_descriptor_t* animation_descriptor );

void callback_server();