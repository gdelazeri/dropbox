#include "dropboxUtil.hpp"
#include "file.hpp"

/* Save users server in database.txt */
void saveUsersServer(std::list<UserServer*> users)
{
    std::fstream file;
    file.open("server/database.txt", std::ios::out);
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

/* Load users server from database.txt */
std::list<UserServer*> loadUsersServer()
{
    std::list<UserServer*> usersDB;
    std::fstream file;
    std::string line; 

    file.open("server/database.txt", std::ios::in);
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

/* Create new ports to be used in sockets */
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

/* Get currente date time */
std::string getCurrentTime(){
    time_t     now = time(0);
    struct tm  tstruct;
    char       timeNow[80];
    tstruct = *localtime(&now);
    strftime(timeNow, sizeof(timeNow), "%Y/%m/%d %H:%M:%S", &tstruct);

    return std::string(timeNow);
}

/* DEBUG */
void printUsersServer(std::list<UserServer*> users)
{
    for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        std::cout << "User: " << (*it)->userid << "\n";
        if (!(*it)->files.empty())
        {
            std::cout << "#files: " << "\n";
            for (std::list<File>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                std::cout << f->pathname << "\t";
                std::cout << f->last_modified << "\t\n";
            }
        }
        std::cout << "\n\n";
	}
}
