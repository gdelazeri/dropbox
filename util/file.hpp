#ifndef __FILE_HPP__
#define __FILE_HPP__

#include <string>
#include <fstream>
#include <cstddef>
#include <sys/stat.h>

class File
{
	public:
		std::string pathname;
		
		std::string name;
		std::string extension;
		std::string last_modified;
		std::string access_time;
		std::string creation_time;
		int size;

		File(void);
		File(std::string pathname);
		const char* getFilename();
		std::string getPath();
		std::string getName();
		std::string getExtension();
		int getSize();
		bool exists();
		std::string getTime(char type);
};
#endif