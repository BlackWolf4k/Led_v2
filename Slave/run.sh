#!/bin/sh
ampy -p /dev/ttyACM0 put Client/client.py
ampy -p /dev/ttyACM0 put Server/server.py
ampy -p /dev/ttyACM0 put Led/led.py
ampy -p /dev/ttyACM0 put shared_informations.py
ampy -p /dev/ttyACM0 put main.py