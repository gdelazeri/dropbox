#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <string>

class Request // : public Communication
{
	public:
		int type;
        std::string argument;
		Request(int type, std::string argument);
};
#endif