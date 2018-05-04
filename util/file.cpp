#include "file.hpp"
#include <iostream>

File::File(std::string pathname)
{
    this->pathname = pathname;
    this->name = this->getName();
    this->extension = this->getExtension();
    this->last_modified = this->getLastModified();
    this->size = this->getSize();
}

const char* File::getFilename()
{
	std::size_t slashIndex = this->pathname.find_last_of("/\\");
	slashIndex = slashIndex > this->pathname.length() ? 0 : slashIndex + 1;
  	
    return this->pathname.substr(slashIndex, this->pathname.length()).c_str();
}

std::string File::getName()
{
	std::size_t slashIndex = this->pathname.find_last_of("/\\");
	std::size_t pointIndex = this->pathname.find_last_of(".\\");
	slashIndex = slashIndex > this->pathname.length() ? 0 : slashIndex + 1;
	pointIndex = pointIndex > this->pathname.length() ? this->pathname.length() : pointIndex;

    return this->pathname.substr(slashIndex, pointIndex-slashIndex).c_str();
}

std::string File::getExtension()
{
	std::size_t pointIndex = this->pathname.find_last_of(".\\");
	pointIndex = pointIndex > this->pathname.length() ? 0 : pointIndex + 1;
  	
    return this->pathname.substr(pointIndex, this->pathname.length()).c_str();
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

bool File::exists() 
{
    struct stat buf;
    return (stat(this->pathname.c_str(), &buf) == 0);
}

std::string File::getLastModified()
{
    struct stat st;
    if (stat(this->pathname.c_str(), &st) == -1) {
        perror("stat");
        return std::string();
    }

    char mtime[80];
    time_t t = st.st_mtime; /*st_mtime is type time_t */
    struct tm lt;
    localtime_r(&t, &lt); /* convert to struct tm */
    strftime(mtime, sizeof mtime, "%Y/%m/%d %H:%M:%S", &lt);

    return std::string(mtime);
}