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
        std::list<std::pair<std::string, int>> deleted;

		UserServer();

		void login();
		void logout();
		
		bool createDir();
		std::string getFolderName();
		std::string getFolderPath();

		// Files
		void addFile(std::string pathname, std::string modificationTime, std::string accessTime, std::string creationTime);
		std::string getFileTime(std::string pathname, char type);
		void removeFile(std::string pathname);
};
#endif