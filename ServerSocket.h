#ifndef Socket_Class
#define Socket_Class

#define MAXLINE 1024

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

class ServerSocket{
    public:
        ServerSocket(int port);
        ~ServerSocket();
        int init();
        int waitforconnection();
        void printinfo();

    
    private: 
        int m_listenfd,m_connfd;
        int m_port;
        char m_read_buf[MAXLINE];
        int m_read_cnt;
        char *m_readptr;
        sockaddr_in m_servaddr,m_clientaddr;
        ssize_t my_read(int fd, char *ptr);
        ssize_t readline(int fd, void *vptr,size_t maxlen);
        ssize_t readlinebuf(void **vptrptr);

        
};

#endif