#include <string>
#include <fstream>
#include <cstddef>
#include <sys/stat.h>

class File
{
	public:
		std::string pathname;
		
		File(std::string pathname);
		const char* getFilename();
		std::string getPath();
		int getSize();
		bool exists();
};