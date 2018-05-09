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
            for (std::list<File>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                file << f->pathname << "\n";
                file << f->last_modified << "\n";
            }
        }
        file << "#end\n";
	}
    file.close();
}

std::list<UserServer*> loadUsersServer()
{
    std::list<UserServer*> usersDB;
    std::fstream file;
    std::string line; 

    file.open("db.txt", std::ios::in);
    while (std::getline(file, line))
    {
        if (line == "#user") 
        {
            // Get user id
            std::getline(file, line);
            UserServer* user = new UserServer();
            user->userid = line;

            // Get pathnames
            std::getline(file, line);
            if (line == "#files")
            {
                while (line != "#end") {
                    std::getline(file, line);
                    if (line != "#end"){
                        File newFile;// = new File();
                        newFile.pathname = line;
                        
                        std::getline(file, line);
                        newFile.last_modified = line;
                        user->files.push_back(newFile);
                    }
                }
            }

            // Add in list
            usersDB.push_back(user);
        }
    }
    file.close();

    return usersDB;
}

void printUsers(std::list<UserServer*> users)
{
    // for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    // {
    //     std::cout << (*it)->userid << "\n";
    //     if (!(*it)->files.empty())
    //     {
    //         std::cout << "#files: " << "\n";
    //         for (std::list<File*>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
    //         {
    //             std::cout << "-\n";
    //             std::cout << (*f)->pathname << "\n";
    //             std::cout << (*f)->name << "\n";
    //             std::cout << (*f)->extension << "\n";
    //             std::cout << (*f)->last_modified << "\n";
    //             std::cout << (*f)->size << "\n";
    //         }
    //     }
    //     std::cout << "\n";
	// }
}

int createNewPort(std::list<int> portsInUse){
	int newPort;
	std::list<int>::iterator findIter;
	
	while(1) {
		newPort = std::rand() % 2000 + SERVER_PORT;
		findIter = std::find(portsInUse.begin(), portsInUse.end(), newPort);
		if (*findIter == (int) portsInUse.size()) {
			portsInUse.push_back(newPort);
			return newPort;
		}
	}
}

/* Parse data ports */
std::pair<int, int> getPorts(char* data) {
	std::string port = std::string(data);
	int p1 = std::stoi(port.substr(0,4));
	int p2 = std::stoi(port.substr(4,8));

	return std::make_pair(p1, p2);
}