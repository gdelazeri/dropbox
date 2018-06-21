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

Request::Request(int type, std::string argument, std::string argument2, int argument3)
{
	this->type = type;
	this->argument = argument;
	this->argument2 = argument2;
	this->argument3 = std::to_string(argument3);
}

Request::Request(int type, std::string argument, std::string argument2, std::string argument3, std::string argument4)
{
	this->type = type;
	this->argument = argument;
	this->argument2 = argument2;
	this->argument3 = argument3;
	this->argument4 = argument4;
}

Request::Request(int type, std::string argument, std::string argument2, std::string argument3, std::string argument4, std::string argument5)
{
	this->type = type;
	this->argument = argument;
	this->argument2 = argument2;
	this->argument3 = argument3;
	this->argument4 = argument4;
	this->argument5 = argument5;
}
