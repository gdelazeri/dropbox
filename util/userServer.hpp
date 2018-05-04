#ifndef __USERSERVER_HPP__
#define __USERSERVER_HPP__

#include <string>
#include <list>
#include "file.hpp"

class UserServer
{
	public:
		int	devices[2];
        std::string userid;
        std::list<File*> files;
		int logged_in;

		void login();
		void logout();
		std::string getFolderName();
		std::string getFolderPath();
		bool createDir(std::string side);
};
#endif