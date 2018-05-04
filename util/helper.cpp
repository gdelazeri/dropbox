#include "helper.hpp"
#include "file.hpp"

void saveUsersServer(std::list<UserServer*> users)
{
    std::fstream file;
    file.open("db.txt", std::ios::out);
    for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        file << "#user\n";
        file << (*it)->userid << "\n";
        if (!(*it)->files.empty())
        {
            file << "#files\n";
            for (std::list<File*>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                file << (*f)->pathname << "\n";
            }
        }
        file << "#end\n";
	}
    file.close();
}

std::list<UserServer*> loadUsersServer()
{
    std::list<UserServer*> users;
    std::fstream file;
    std::string line; 

    file.open("db.txt", std::ios::in);
    while (std::getline(file, line))
    {
        if (line == "#user") 
        {
            std::getline(file, line);
            UserServer* user = new UserServer();
            user->userid = line;

            std::getline(file, line);
            if (line == "#files")
            {
                while (line != "#end") {
                    std::getline(file, line);
                    if (line != "#end"){
                        File* file = new File(line);
                        user->files.push_back(file);
                    }
                }
            }
            users.push_back(user);
        }
    }
    file.close();

    return users;
}

void printUsers(std::list<UserServer*> users)
{
    for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        std::cout << (*it)->userid << "\n";
        if (!(*it)->files.empty())
        {
            std::cout << "#files: " << "\n";
            for (std::list<File*>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                std::cout << "-\n";
                std::cout << (*f)->pathname << "\n";
                std::cout << (*f)->name << "\n";
                std::cout << (*f)->extension << "\n";
                std::cout << (*f)->last_modified << "\n";
                std::cout << (*f)->size << "\n";
            }
        }
        std::cout << "\n";
	}
}
