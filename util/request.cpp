#include "request.hpp"

Request::Request(int type, std::string argument)
{
	this->type = type;
	this->argument = argument;
}

Request::Request(int type, std::string argument, std::string argument2)
{
	this->type = type;
	this->argument = argument;
	this->argument2 = argument2;
}

