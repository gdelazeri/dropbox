#define SERVER_PORT 4000
#define SERVER_NAME "[server@dropbox] "
#define CLIENT_NAME "[client@dropbox] "

/* Socket side definition */
#define SOCK_CLIENT 0
#define SOCK_SERVER 1

/* Datagram Types */
#define LOGIN_TYPE 1

/* Datagram definition */
#define ERROR 'E'
#define LOGIN 'L'
#define NEW_PORTS 'P'
#define BEGIN_FILE_TYPE 'I'
#define FILE_TYPE 'F'
#define END_FILE_TYPE 'N'
#define BUFFER_SIZE 6000
#define MAX_DATA_SIZE 5999

#define UPLOAD_REQUEST 1

typedef struct tDatagram
{
  char type;
  char data[MAX_DATA_SIZE];
} tDatagram;


// #include "userServer.hpp"
// void saveUsersServer(std::list<UserServer> users);
// std::list<UserServer> loadUsersServer();