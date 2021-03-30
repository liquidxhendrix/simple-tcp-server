#include "ServerSocket.h"
#include <iostream>
#include <errno.h>
using namespace std;

#define SA struct sockaddr

ServerSocket::ServerSocket(int port)
{
    m_listenfd=0;
    m_connfd=0;
    m_port=port;
    m_read_cnt=0;
    bzero(&m_servaddr,sizeof(m_servaddr));
    bzero(&m_clientaddr,sizeof(m_clientaddr));
}

ServerSocket::~ServerSocket(){
    close(m_connfd);
    close(m_listenfd);
}

int ServerSocket::init(){
    //1. Create Socket

    m_listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (m_listenfd<0)
    {
        cout<< "Create Socket Failed. Error:" <<errno <<"\n";
        return -1;
    }else
    {
        cout << "Socket started on port " << m_port <<"\n";
    }

    //2. Bind

    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on all network interfaces
    m_servaddr.sin_port = htons(m_port);

    if (bind(m_listenfd, (SA *)&m_servaddr,sizeof(m_servaddr)) <0)
    {
        cout<< "Bind Socket Failed Errorno:"<<errno <<"\n";
        return -1;
    }else
    {
        cout << "bind successful on port" << m_port<<"\n";
    }


    //3. Listen
    if (listen(m_listenfd, 10) <0) //set backlog to 10 for testing
    {
        cout<< "Listen Socket Failed. Errorno:"<<errno <<"\n";;
        return -1;
    }
    else
    {
        cout << "listening on port" << m_port<<"\n";
    }

    return 0;
}

int ServerSocket::waitforconnection(){
    
    //1. Go to loop and accept
    // TODO: Convert to SELECT multiplexed I/O
    socklen_t clilen;
    char buff[MAXLINE];

    for ( ; ; ){
        clilen = sizeof(m_clientaddr);

        if (m_connfd ==0){
            //Wait for incoming connection
            m_connfd = accept(m_listenfd, (SA *)&m_clientaddr, &clilen);
        }

        if (m_connfd <1)
        {
            cout<< "Accept incoming connection failed. Errorno:"<<errno <<"\n";
            return -1;
        }else
        {
            //Accepted
            cout<< "Connection from IP:"<< inet_ntop(AF_INET,&m_clientaddr.sin_addr.s_addr,buff,sizeof(buff)) <<" Port:"<< ntohs(m_clientaddr.sin_port) <<"\n";

            //Read from socket
            for ( ; ; )   
            {
                if (0>=readline(m_connfd,(void*) buff, sizeof (buff)))
                {
                    //Error or client terminated. return to main
                    cout << "Error or client terminated.\n";
                    close(m_connfd);
                    m_connfd=0; //Wait for connection again
                    break;
                }

                //echo to terminal

                cout << "Received:"<<buff;
            }

        }


    }

    //2. Accept & echo data to terminal
    // TODO: Draw a shape on screen using openCV

    return 0;

}

void ServerSocket::printinfo(){

}

ssize_t ServerSocket::my_read(int fd, char *ptr){

    if (m_read_cnt <= 0){
        //No data read from socket yet, read in MAXLINE blocks
        again:
        if ((m_read_cnt = read(fd,m_read_buf,sizeof(m_read_buf))) < 0){
            if (EINTR == errno){
                //Interrupted by SIGNAL, continue
                    goto again;
            return (-1); //Real error
            }
        }else if (0 == m_read_cnt){
            return (0);
        }

        //Block read successful, point to head of buffer
        m_readptr = m_read_buf;
    }

    //decrement readcnt by 1, and copy to calling destination memory
    //successive calls will not read from socket by return values from internal buffer
    //once m_read_cnt is 0, will read from the socket again from the loop above
    m_read_cnt--; 
    *ptr = *m_readptr++;
    return (1); 
}

ssize_t ServerSocket::readline(int fd, void *vptr,size_t maxlen){
    ssize_t n,rc;
    char    c,*ptr;

    ptr=(char*)vptr;

    for (n=1; n<maxlen ; n++){
        if ((rc = my_read(fd, &c) == 1)){
            //Successfully read 1 char
            *ptr++ = c;
            if (c == '\n')
                break; //Newline is hit
        }else if (0 == rc){
            return (0); //Client is closed
        }else
            return (-1);    //Error
    }

    *ptr = 0; //Append null at the end
    return (n);
}

ssize_t ServerSocket::readlinebuf(void **vptrptr){
    if (0<m_read_cnt){
        *vptrptr=m_readptr;
    }

    return m_read_cnt;
}
