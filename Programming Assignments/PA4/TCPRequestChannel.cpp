#include "TCPRequestChannel.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
//have types include in header file (which is then in common.h)
using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    //check if server or client
    if(_ip_address.empty()){    
        //server
        //get IP address of current machine
        struct addrinfo hints, *res, *p;
        int status;
        char ipstr[INET6_ADDRSTRLEN];
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET; //IPv4
        hints.ai_socktype = SOCK_STREAM; //TCP
        hints.ai_flags = AI_PASSIVE; //fill in my IP for me
        hints.ai_protocol = 0; //default protocol
        if((status = getaddrinfo("localhost",NULL, &hints, &res)) != 0){
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            exit(1);
        }
        //iterate through linked list of results and return the first one
        for(p = res; p != NULL; p = p->ai_next){
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            void* addr = &(ipv4->sin_addr);
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        }
        //print IP address for confirmation
        std::cout << "IP address for localhost: " << ipstr << std::endl;
        //free linked list
        freeaddrinfo(res);


        //using sockaddr_in struct cause its easier to deal with for IPv4
        struct sockaddr_in server;
        server.sin_family = AF_INET; //IPv4
        server.sin_port = htons(stoi(_port_no)); //convert string to int to short (host to network short)
        inet_pton(AF_INET, ipstr, &(server.sin_addr)); //convert IP address from text to binary form
        //create socket - socket(int domain, int type, int protocol)
        //we want AF_INET domain (IPv4), SOCK_STREAM type (TCP), and 0 protocol (default)
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

        //bind the socket to assign address to socket to allow server to listen on _port_no
        bind(sockfd, (struct sockaddr *)&server, sizeof(server));
        //market socket as listening
        //can have the backlog be a somewhat big number like 15
        listen(sockfd, 15);
    }
    else{
        //client
        //almost the same thing I did in the server, just connecting this time 
        //need the sockaddr_in struct to connect to the server
        struct sockaddr_in client;
        client.sin_family = AF_INET; //IPv4
        client.sin_port = htons(stoi(_port_no)); //convert string to int to short (host to network short)
        inet_pton(AF_INET, _ip_address.c_str(), &(client.sin_addr)); //convert IP address from text to binary form
        //create socket with domain, type, and protocol (return socket file descriptor)
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        //connect socket to the IP address of the server
        connect(sockfd, (struct sockaddr *)&client, sizeof(client));
    }
}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    sockfd = _sockfd; //just pass the sockfd lol
}

TCPRequestChannel::~TCPRequestChannel () {
    //close the socket file descriptor
    //close(int fd)
    close(sockfd);
}

int TCPRequestChannel::accept_conn () {
    //implementing accept(...)
    //return value of this function is the socket file descriptor of client
    //accept the connection - accept(int sockfd, struct sockaddr *addr, socken_t *addrlen)
    //socket file descriptor for accepted connection
    //return socket file descriptor
    int socketFD;
    socketFD = accept(sockfd, NULL, NULL); //just need this function and then return its socket FD
    return socketFD;
}

//can use read/write or send/recv
int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    //read from socket - read(int fd, void *buf, size_t count)
    //return the number of bytes read
    //can just use the read function from the previous PA
    return read(sockfd, msgbuf, msgsize);

}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    //write to socket - write(int fd, const void *buf, size_t count)
    //return the number of bytes written
    //can just use the write function from the previous PA
    return write(sockfd, msgbuf, msgsize);
}
