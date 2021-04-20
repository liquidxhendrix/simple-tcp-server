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
    m_echomode=ECHO_MODE_STDOUT;
    bzero(&m_servaddr,sizeof(m_servaddr));
    bzero(&m_clientaddr,sizeof(m_clientaddr));
}

ServerSocket::~ServerSocket(){
    close(m_connfd);
    close(m_listenfd);
}

int ServerSocket::init(){
    //1. Create Socket

    const int on =1;

    m_listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (m_listenfd<0)
    {
        cout<< "Create Socket Failed. Error: " <<errno <<"\n";
        return -1;
    }else
    {
        cout << "Socket started on port " << m_port <<"\n";
    }

    //2. Bind

    bzero(&m_servaddr,sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on all network interfaces
    m_servaddr.sin_port = htons(m_port);

    //Added to allow re-use of port during 4-way closing handshake
    if (setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) <0)
    {
        cout<< "setsockopt Failed but continuing Errorno: "<<errno <<"\n";
    }


    if (bind(m_listenfd, (SA *)&m_servaddr,sizeof(m_servaddr)) <0)
    {
        cout<< "Bind TCP Socket Failed Errorno: "<<errno <<"\n";
        return -1;
    }else
    {
        cout << "TCP bind successful on port " << m_port<<"\n";
    }


    //3. Listen
    if (listen(m_listenfd, 10) <0) //set backlog to 10 for testing
    {
        cout<< "Listen TCP Socket Failed. Errorno: "<<errno <<"\n";;
        return -1;
    }
    else
    {
        cout << "TCP listening on port " << m_port<<"\n";
    }

    // 4. Init UDP port

    m_udpfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    //Bind

    bzero(&m_servaddr,sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on all network interfaces
    m_servaddr.sin_port = htons(m_port);

    if (bind(m_udpfd, (SA *)&m_servaddr,sizeof(m_servaddr)) <0)
    {
        cout<< "Bind UDP Socket Failed Errorno: "<<errno <<"\n";
        return -1;
    }else
    {
        cout << "UDP bind successful on port " << m_port<<"\n";
    }




    return 0;
}

void ServerSocket::init_monitoring(){
    m_maxfd = max(m_listenfd,m_udpfd);
    m_maxi = -1; //Client array index

    for (m_i =0;m_i < MAX_CLIENTS; m_i++)
        m_client[m_i] = -1;

    FD_ZERO(&m_allset);
    FD_SET(m_listenfd,&m_allset); // Add the TCPlistening FD into the base fdset
    FD_SET(m_udpfd,&m_allset); // Add the UDP listening FD into the base fdset
}

int ServerSocket::add_connfd_to_monitoring(int connfd){
     
    
     for (m_i=0; m_i < MAX_CLIENTS; m_i++)
        if (m_client[m_i] < 0){
            m_client[m_i] = connfd;
            break;
        }
            
    if (m_i == MAX_CLIENTS)
        m_i=-1;
    else{
        FD_SET(connfd,&m_allset);

        if (connfd>m_maxfd)
            m_maxfd = connfd;

        if (m_i > m_maxi)
            m_maxi = m_i;
    }

    return m_i;

}

int ServerSocket::waitForConnection(){
    
    //1. Go to loop and accept
    socklen_t clilen;
    char buff[MAX_LINE];

    //Setup for multiplex
   
    init_monitoring(); //init the monitoring client array

    for ( ; ; ){

        //Multiplex on select call
        m_rset = m_allset;
        m_nready = select(m_maxfd + 1, &m_rset , NULL, NULL, NULL);
        //Block until connection or read/write is activated on FD SET

        if (FD_ISSET(m_listenfd,&m_rset))
        {
            //incoming connection
            clilen = sizeof(m_clientaddr);

            m_connfd = accept(m_listenfd, (SA *)&m_clientaddr, &clilen);
        
            if (m_connfd<1)
            {
                cout<< "Accept incoming connection failed. Errorno:"<<errno <<"\n";
                return -1;
            }else
            {
                //Accepted. Add connection FD to monitoring array and continue listening

                if (0>add_connfd_to_monitoring(m_connfd))
                {
                    cout << "Connection failed (Max clients connected)\n";
                    close(m_connfd);
                    m_connfd=0; //Wait for connection again
                    break;
                }

                cout<< "Connection from IP:"<< inet_ntop(AF_INET,&m_clientaddr.sin_addr.s_addr,buff,sizeof(buff)) <<" Port:"<< ntohs(m_clientaddr.sin_port) <<"\n";

                if (--m_nready <= 0)
                    continue; //No more descriptors that are ready
            }

               
            
        }

        //Added for UDP server process
        if (FD_ISSET(m_udpfd,&m_rset))
        {
            //data received on UDP port
            int n=0;

            clilen = sizeof(m_clientaddr);

            n=recvfrom(m_udpfd,(void*) buff, sizeof (buff),0,(SA *)&m_clientaddr,&m_len);

                    cout << "\n-------------------------------\n";
                    cout << "Received (UDP):"<<buff<<"\n";

                    switch(m_echomode){

                        case ECHO_MODE_STDOUT:

                        break;

                        case ECHO_MODE_SERV:
                        default:
                            //Convert to shapestype and dump data
                            ShapeType shapeConverted;
                            long temp_x;

                            JSON_to_shapes(buff,strlen(buff),&shapeConverted);

                            cout<< "ShapesType Struct:\tcolor:"<<shapeConverted.color<<" x="<<shapeConverted.x<<" y="<<shapeConverted.y<<" shapesize="<<shapeConverted.shapesize<<"\n";

                            cout << "Swapping X,Y and echoing back...\n";

                            temp_x = shapeConverted.x;
                            shapeConverted.x = shapeConverted.y;
                            shapeConverted.y = temp_x;
                            
                            cout<< "ShapesType Struct:\tcolor:"<<shapeConverted.color<<" x="<<shapeConverted.x<<" y="<<shapeConverted.y<<" shapesize="<<shapeConverted.shapesize<<"\n";
                            shapes_to_JSON(&shapeConverted,buff,MAX_LINE);

                            //send reply
                            cout << "Echoing back to server in 1 s...\t\n";
                            sleep(1);
                            cout << "Sending (UDP):\n"<<buff<<"\n";
                            sendto(m_udpfd,(void*) buff, strlen(buff)+1,0,(SA *)&m_clientaddr,m_len);
                            cout << "\n-------------------------------\n";
                            break;
                     }

                    if (--m_nready <= 0)
                    continue; //No more descriptors that are ready
       
        }

        //Not new connection or UDP packet. Loop to process existing client connections
        for ( m_i=0; m_i<=m_maxi;m_i++)   
        {

            if ( (m_sockfd = m_client[m_i]) <0)
                continue; //Empty slot

            if (FD_ISSET(m_sockfd,&m_rset)){//--> Read and output to terminal

                if (0>=readline(m_sockfd,(void*) buff, sizeof (buff)))
                {
                    //Error or client terminated. return to main
                    cout << "Error or client terminated.\n";
                    FD_CLR(m_sockfd,&m_allset);
                    close(m_sockfd);
                    m_client[m_i]=-1;
                    m_connfd=0; //Wait for connection again
                }else
                {
                    // 2. Accept & echo data depending on mode
                    // TODO: Draw a shape on screen using openCV
                    cout << "\n-------------------------------\n";
                    cout << "Received (TCP):"<<buff<<"\n";

                    switch(m_echomode){

                        case ECHO_MODE_STDOUT:
                    
                        break;

                        case ECHO_MODE_SERV:
                        default:
                            //Convert to shapestype and dump data
                            ShapeType shapeConverted;
                            long temp_x;

                            JSON_to_shapes(buff,strlen(buff),&shapeConverted);

                            cout<< "ShapesType Struct:\tcolor:"<<shapeConverted.color<<" x="<<shapeConverted.x<<" y="<<shapeConverted.y<<" shapesize="<<shapeConverted.shapesize<<"\n";

                            cout << "Swapping X,Y and echoing back...\n";

                            temp_x = shapeConverted.x;
                            shapeConverted.x = shapeConverted.y;
                            shapeConverted.y = temp_x;
                            
                            cout<< "ShapesType Struct:\tcolor:"<<shapeConverted.color<<" x="<<shapeConverted.x<<" y="<<shapeConverted.y<<" shapesize="<<shapeConverted.shapesize<<"\n";
                            shapes_to_JSON(&shapeConverted,buff,MAX_LINE);

                            //send reply
                            cout << "Echoing back to server in 1 s...\t\n";
                            sleep(1);
                            cout << "Sending (TCP):\n"<<buff<<"\n";
                            writen(m_sockfd,(void*) buff, strlen(buff)+1);
                            cout << "\n-------------------------------\n";
                            break;
                    }
                    
                }

                if (--m_nready <= 0)
                    break; //No more descriptors that are ready, break out of loop

            } //<<- Read and output to terminal
        }
    }

    

    return 0;

}

void ServerSocket::printinfo(){

}

ssize_t ServerSocket::my_read(int fd, char *ptr){

    if (m_read_cnt <= 0){
        //No data read from socket yet, read in MAX_LINE blocks
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
            if (c == '\0')
                break; //End string is hit
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

ssize_t ServerSocket::writen(int fd, const void *vptr, size_t n){
   //Write the whole string in a loop
   
   size_t nleft;
   ssize_t nwritten;
   const char *ptr;

   ptr = (char*) vptr;
   nleft = n;
   while (nleft > 0){
      if (  (nwritten = write(fd,ptr,nleft)) <=0){
         if (nwritten < 0 && errno == EINTR) //interrupted
            nwritten = 0; 
         else
            return (-1); //error happened
      }

      nleft -= nwritten;
      ptr += nwritten;
   }

   return(n);
}

void ServerSocket::setEchoModeStdOut()
{
    m_echomode = ECHO_MODE_STDOUT;
}

void ServerSocket::setEchoModeServer()
{
    m_echomode = ECHO_MODE_SERV;
}