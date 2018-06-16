#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__

#include <string.h>
#include "userServer.hpp"

class Device
{
	public:
		int connected;
        UserServer* user;
		std::string address;
		int port;

        Device(UserServer* user, std::string address, int port);
		bool connect();
		bool disconnect();
};
#endif