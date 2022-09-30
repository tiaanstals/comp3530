CC=gcc 
CFLAGS=-std=c99 -Wall

default: traffic_lights

traffic_lights: traffic_lights.c functions.c
	$(CC) $(CFLAGS) traffic_lights.c functions.c -o traffic_lights 

clean:
	rm -f traffic_lights
