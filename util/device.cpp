#include "device.hpp"
#include <iostream>

Device::Device(UserServer* user, std::string address, int port)
{
	this->user = user;
	this->address = address;
	this->port = port;
}

/* Set flag to connected and control user devices limit */
bool Device::connect()
{
	if (this->user->devices < 2)
	{
		this->user->devices++;
		this->connected = 1;
		return true;
	}
	else 
	{
		return false;
	}
}

/* Set flag to disconnected and control user devices limit */
bool Device::disconnect()
{
	if (this->user->devices > 0)
	{
		this->user->devices--;
		this->connected = 0;
		return true;
	}
	else 
	{
		return false;
	}
}
