/*
 * stk_io.cpp
 *
 * Created: 18-11-2017 15:20:29
 *  Author: JMR_2
 */ 

// Includes
#include "stk_io.h"
#include "WiFiServer.h"
#include "WiFiClient.h"

static WiFiClient client;
static WiFiServer wifiServer(80);

// Functions
uint8_t STK_io::put(char c) {
	// write on socket
	if (client.connected()) {
		return client.write(c);
	}
	return 0;
}

uint8_t STK_io::get() {
	// read from socket
	if (client.connected()) {
		while (!client.available()) {}
		return client.read();
	}
	return 0;
}

void STK_io::init(void)
{
	// open socket on port 80
	wifiServer.begin();
	while (!client.connected()) {
		client = wifiServer.available();
	}
}
