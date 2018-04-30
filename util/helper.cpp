#include "helper.hpp"
#include <list>

void saveUsersServer(std::list<UserServer> users)
{
    std::fstream file;
    file.open("db.txt", std::ios::out);
    for (std::list<UserServer>::iterator it = users.begin(); it != users.end(); ++it)
    {
        file << it->userid << "\n";
	}
    file.close();
}

std::list<UserServer> loadUsersServer()
{
    std::list<UserServer> users;
    std::fstream file;
    std::string line; 

    file.open("db.txt", std::ios::in);
    while (std::getline(file, line))
    {
        UserServer user;
        user.userid = line;
        users.push_back(user);
    }
    file.close();

    return users;
}