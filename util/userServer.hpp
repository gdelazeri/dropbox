#include <string>
#include <list>

class UserServer
{
	public:
		int	devices[2];
        std::string userid;
        // File files[10];
		int logged_in;

		void login();
		void logout();
		std::string getFolderName();
		std::string getFolderPath();
		bool createDir();
};
