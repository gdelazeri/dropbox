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
#define FILE 2
#define BUFFER_SIZE 1024
#define MAX_DATA_SIZE 1023

typedef struct tDatagram
{  
  char type;
  char data[MAX_DATA_SIZE];
} tDatagram;