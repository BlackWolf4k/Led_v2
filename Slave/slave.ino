#include <ESP8266WiFi.h>
// #include <Adafruit_NeoPixel.h>

/*
ANIMATION DESCRIPTOR
number_of_line: 	the number of lines in the file
line_length:		the length of a single line
repeat: 			does the animation repeat ( 0 - 254: number of times to repeat, 255: loop )
colors: 			pointer to the colors matrix
delays: 			pointer to the delays matrix
*/
struct animation_descriptor_t
{
	int number_of_lines;
	int line_length;
	byte repeat;
	byte* colors;
	byte* delays;
};

/*
SLAVE CONNECTION DESCRIPTOR
id: 			the id of the slave
ip_address: 	the ip address of the slave
*/
struct slave_connection_t
{
	uint8_t id;
	char ip_address[16];
};

// Start the connection with the main server
// Send basic informations about the slave, passed by argument
// Return a status code where the sending was sucessfull ( -1: error )
byte send_slave_connection_informations( slave_connection_t slave_informations );

// Recive the animation descriptor from the main server
// Requires a argument where to store the animation descriptor
// Return a status code where the reciving was sucessfull ( -1: error )
byte recive_animation_descriptor( animation_descriptor_t* animation_descriptor );

// Recive the animation colors and delays from the main server
// Requires the animation descriptor as argument
// Return a status code where the reciving was sucessfull ( -1: error )
byte recive_animation( animation_descriptor_t* animation_descriptor );

// Play the animation
// Requires as argument the animation descriptor
// Returns a status code ( 0: animation ended, 1: main server forced to quit, -1: error )
byte play_animation( animation_descriptor_t* animation_descriptor );

// The id of the board, this should be changed for every slave
#define ID 1

// Informations about the wifi
char* ssid = "led";
char* password = "password";

// Informaions aboud the server
#define PORT 1024

// Local machine IP address
IPAddress ip_address( 192, 168, 0, 2 );
IPAddress gateway( 192, 168, 0, 1 );
IPAddress subnet_mask( 255, 255, 255, 0 );

// Create a server listening on slave port
WiFiServer server( PORT );

// Create a client to comunicata with the main server
WiFiClient client;

void setup()
{
	// Begin the serial
	Serial.begin( 115200 );

	// Configure the leds

	// Check that the static addressing was sucessfull
	WiFi.mode( WIFI_STA );

	// Connect to the server's wifi
	WiFi.begin( ssid, password );

	// Wait untill slave is connected
	while ( WiFi.status() != WL_CONNECTED )
	{
		Serial.println( "Waiting for connection" );
		delay( 500 );
	}
}

void loop()
{
	// Stores return values from send and recive functions
	byte return_value = 0;

	// Connect the client
	client.connect( gateway, PORT );

	// Intialize the infromation to send to the main server
	slave_connection_t slave_informations;
	slave_informations.id = ID;
	strcpy( slave_informations.ip_address, "192.168.0.2" );

	Serial.println( "Sending basic informations" );

	// Send the slave informaions to the main server
	return_value = send_slave_connection_informations( slave_informations );

	// Check that the connection was sucessfull
	if ( return_value == -1 )
		Serial.println( "There was an error in the connection" );

	// Create a animation descriptor to store the informations
	animation_descriptor_t animation_descriptor;

	Serial.println( "Reciving animation descriptor" );

	// Recive the animation descriptor from the main server
	return_value = recive_animation_descriptor( &animation_descriptor );

	Serial.println( "Reciving animation" );

	// Recive the animation from the main server
	return_value = recive_animation( &animation_descriptor );

	Serial.println( "Closing connection with main server" );

	// Close the connection with the main server
	client.stop();

	Serial.println( "Starting playing animation" );

	// Start playing the animation
	play_animation( &animation_descriptor );
}

byte send_slave_connection_informations( slave_connection_t slave_informations )
{
	// Send the slave connection descriptor
	int bytes_sent = 0;
	for ( byte i = 0; i < sizeof( slave_connection_t ); i++ )
		bytes_sent += client.write( ( ( byte* )( &slave_informations ) ), sizeof( slave_connection_t ) ); // Count the number of lines sent

	// Check that the sending was sucessfull
	if ( bytes_sent != sizeof( slave_connection_t ) )
		return -1; // Return failure status

	// Recive main server ack message
	byte ack = client.read();

	// Check the ack message to check that everything was ok
	// 0: not ok, 1: ok
	if ( !ack )
		return -1; // Return failure status
	
	return 0; // Everything was fine
}

byte recive_animation_descriptor( animation_descriptor_t* animation_descriptor )
{
	// Recive animation descriptor
	for ( int i = 0; i < sizeof( animation_descriptor_t ); i++ )
		( ( byte* )( &animation_descriptor ) )[i] = client.read(); // Alternative way to read without a buffer
	
	return 0; // Everything was fine
}

byte recive_animation( animation_descriptor_t* animation_descriptor )
{
	// Create animations and delays buffers
	// Created as static to make them persist after that the scope finishes
	static byte* colors = ( byte* )malloc( animation_descriptor -> number_of_lines / 2 * animation_descriptor -> line_length );
	static byte* delays = ( byte* )malloc( animation_descriptor -> number_of_lines / 2 * animation_descriptor -> line_length / 3 );

	// Store the arrays in the animation descriptor
	animation_descriptor -> colors = colors;
	animation_descriptor -> delays = delays;

	// Recive the animation informations and store them in the buffers
	for ( int i = 0; i < animation_descriptor -> number_of_lines; i++ )
	{
		if ( i % 2 == 0 ) // If line is even is a dalay line
			for ( int j = 0; j < animation_descriptor -> line_length / 3; j++ )
				delays[ i * animation_descriptor -> line_length / 3 + j ] = client.read();
		else // If line is odd is a color line
			for ( int j = 0; j < animation_descriptor -> line_length; j++ )
				colors[ i * animation_descriptor -> line_length + j ] = client.read();
	}

	return 0; // Everything was fine
}

byte play_animation( animation_descriptor_t* animation_descriptor )
{}