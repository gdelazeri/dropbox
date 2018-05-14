#ifndef __USERSERVER_HPP__
#define __USERSERVER_HPP__

#include <string>
#include <list>
#include "file.hpp"

class UserServer
{
	public:
		int	devices;
        std::string userid;
        std::list<File> files;

		void login();
		void logout();
		std::string getFolderName();
		std::string getFolderPath();
		bool createDir();

		// Files
		void addFile(std::string pathname, std::string modificationTime);
		std::string getFileModificationTime(std::string pathname);
};
#endif