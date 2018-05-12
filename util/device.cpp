#include "device.hpp"
#include <iostream>

Device::Device(UserServer* user)
{
	this->user = user;
}

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
