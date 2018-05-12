#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__

#include "userserver.hpp"

class Device
{
	public:
		int connected;
        UserServer* user;

        Device(UserServer* user);
		bool connect();
		bool disconnect();
};
#endif