#define BUFFER_SIZE 256
#define UPLOAD "upload"
#define USER "delazeri"
#define ADDRESS "localhost"
#define PORT 4000

struct tDatagram
{  
  int type;
  char buffer[256];
};
  
typedef struct tDatagram tDatagram;

/* Datagram Types */
#define LOGIN_TYPE 1