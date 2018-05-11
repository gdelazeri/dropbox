#include "file.hpp"
#include <iostream>

File::File(void)
{
}

File::File(std::string pathname)
{
    this->pathname = pathname;
    this->name = this->getName();
    this->extension = this->getExtension();
    this->last_modified = this->getTime('M');
    this->access_time = this->getTime('A');
    this->creation_time = this->getTime('C');
    this->size = this->getSize();
}

std::string File::getFilename()
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

    return this->pathname.substr(slashIndex, pointIndex-slashIndex);
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

std::string File::getTime(char type)
{
    struct stat st;
    if (stat(this->pathname.c_str(), &st) == -1) {
        perror("stat");
        return std::string();
    }

    char datetime[80];
    time_t t;
    struct tm lt;

    if (type == 'M') // Last modification
        t = st.st_mtime;
    else if (type == 'A') // Last access
        t = st.st_atime;
    else if (type == 'C') // Last status change
        t = st.st_ctime;
    
    localtime_r(&t, &lt);
    strftime(datetime, sizeof datetime, "%Y/%m/%d %H:%M:%S", &lt);

    return std::string(datetime);
}

int File::getInode(std::string path)
{
    struct stat var;
    int ret = stat((path+this->filename).c_str(), &var);

    if(ret < 0)
        return -1;
    else
        return var.st_ino;
}