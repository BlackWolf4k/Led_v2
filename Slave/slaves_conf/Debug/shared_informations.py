# Value set by main server
# Is array to be shared as ref to threads
# 0: go, 1: get new animation, 2: turn off untill other request, 3 shut down
server_callback = []

# General purpose infromations
generals = {
	"id" : 1,
	"control_pin" : 0, "number_of_leds": 4,
	"ssid" : "SmartLeds", "password" : "password",
	"client_port" : 1234, "server_port" : 1233,
	"slave_password" : "helloworld"
}