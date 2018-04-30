#include "request.hpp"

Request::Request(int type, std::string argument)
{
	this->type = type;
	this->argument = argument;
}
