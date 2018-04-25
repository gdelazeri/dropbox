#define SERVER_PORT 4000
#define SERVER_NAME "[server@dropbox] "
#define CLIENT_NAME "[client@dropbox] "

/* Socket side definition */
#define SOCK_CLIENT 0
#define SOCK_SERVER 1

/* Datagram Types */
#define LOGIN_TYPE 1

/* Datagram definition */
#define LOGIN 0
#define NEW_PORTS 1
#define FILE 2
#define BUFFER_SIZE 1024
#define MAX_DATA_SIZE 1016

struct tDatagram
{  
  int type;
  int dataSize;
  char data[MAX_DATA_SIZE];
};
  
typedef struct tDatagram tDatagram;
