#ifndef Socket_Class
#define Socket_Class

#define MAX_LINE 1024
#define MAX_CLIENTS 32

//for echo mode
#define ECHO_MODE_STDOUT 0
#define ECHO_MODE_SERV 1

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "shapetype.h"
#include "json_helper.h"

class ServerSocket{
    public:
        ServerSocket(int port);
        ~ServerSocket();
        int init();
        int waitforTCPconnection();
        void printinfo();
        void setEchoModeStdOut();
        void setEchoModeServer();

    
    private: 
        int m_listenfd,m_connfd, m_sockfd;
        int m_maxfd, m_i, m_maxi;
        int m_port;
        char m_read_buf[MAX_LINE];
        int m_read_cnt;
        //For multiplexed I/O
        fd_set m_rset,m_allset;
        int m_nready,m_client[MAX_CLIENTS];
        char *m_readptr;
        sockaddr_in m_servaddr,m_clientaddr;
        //For Echo
        int m_echomode;
        ssize_t my_read(int fd, char *ptr);
        ssize_t readline(int fd, void *vptr,size_t maxlen);
        ssize_t readlinebuf(void **vptrptr);   
        void init_monitoring();  
        int add_connfd_to_monitoring(int connfd);    
        ssize_t writen(int fd, const void *vptr, size_t n);
};

#endif