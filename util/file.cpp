#include "file.hpp"

File::File(std::string pathname)
{
    this->pathname = pathname;
}

const char* File::getFilename()
{
	std::size_t slashIndex = this->pathname.find_last_of("/\\");
	slashIndex = slashIndex > this->pathname.length() ? 0 : slashIndex + 1;
  	
    return this->pathname.substr(slashIndex, this->pathname.length()).c_str();
}

std::string File::getPath()
{
    return "";
}

int File::getSize()
{
	std::fstream file;

    file.open(this->pathname.c_str(), std::ios::binary | std::ios::in);
	file.seekg(0, file.end);
	int fileSize = file.tellg();
    file.close();

    return fileSize;
}

bool File::exists() {
    struct stat buf;
    return (stat(this->pathname.c_str(), &buf) == 0);
}
