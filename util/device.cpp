#include "device.hpp"

Device::Device(UserServer* user)
{
	this->user = user;
}

void Device::connect()
{
	this->connected = 1;
}

void Device::disconnect()
{
	this->connected = 0;
}
