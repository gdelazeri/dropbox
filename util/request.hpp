// #include "../util/communication.hpp"
#include <string>

class Request // : public Communication
{
	public:
		int type;
        std::string argument;
		Request(int type, std::string argument);
};

