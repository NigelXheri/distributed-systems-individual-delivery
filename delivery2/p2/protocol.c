/*
MECHANISM: P2P
FILE: protocol.c
DESCRIPTION: This file implements some functions that simplify creation of passive or active sockets, and also various specific functions for the protocol between client and server chat, which allow sending and receive 1 byte of information, a short, a string of given length or a sockaddr_in structure containing IP and port of a contact
*/

#include "protocol.h"

// Close a socket
int Close(int socket)
{
    if (close(socket) != 0)
    {
        perror("When connecting");
        exit(1);
    }
    return 0;
}

/// Create and return a data socket
int CreateTCPDataSocket()
{
    int sock;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("When creating socket");
        exit(-1);
    }
    return sock;
}

// Create a listening socket
int ListeningSocket(int port)
{
    int listening_socket;
    struct sockaddr_in address;

    // Create listening/passive socket (TCP)
    listening_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (listening_socket == -1)
    {
        perror("Error when creating the passive socket");
        exit(-1);
    }


    
    // ADDED Nigel Xherimeja
    int reuse = 1;
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
                (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    #ifdef SO_REUSEPORT
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEPORT,
                (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEPORT) failed");
    #endif



    // Asign IP and port to the socket
    address.sin_family = AF_INET;                // Address type
    address.sin_port = htons(port);              // Local IP
    address.sin_addr.s_addr = htonl(INADDR_ANY); // Port
    if (bind(listening_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("Error when assigning the address to the ps");
        Close(listening_socket);
        exit(-1);
    }

    // Socket in listening/passive mode
    if (listen(listening_socket, SOMAXCONN) == -1)
    {
        perror("Error when setting listening mode");
        Close(listening_socket);
        exit(-1);
    }

    return listening_socket;
}

// Accept a connection on the passive socket, or abort execution on error
int Accept(int listening_socket)
{
    int data_socket;
    struct sockaddr_in address;
    static socklen_t len = sizeof(address);

    data_socket = accept(listening_socket, (struct sockaddr *)&address, &len);
    if (data_socket == -1)
    {
        perror("Error when accepting a client");
        exit(-1);
    }

    return data_socket;
}

// Attempt a connection to the given IP and port(the IP is expected as a
// character string, eg: "127.0.0.1")
// Abort the execution of the program in case of error
int Connect(int socket, char *ip, int port)
{
    struct sockaddr_in dir_serv;
    int result;

    dir_serv.sin_family = AF_INET;
    dir_serv.sin_port = htons(port);
    result = inet_aton(ip, &dir_serv.sin_addr);
    if (result == 0)
    {
        perror("When converting IP");
        exit(1);
    }

    result = connect(socket, (struct sockaddr *)&dir_serv, sizeof(dir_serv));
    if (result == -1)
    {
        perror("When connecting");
        exit(1);
    }
    return result;
}

// ==========================================================================
// Data transmission in binary
//
// The following functions implement sending or receiving through
// of a TCP socket that they receive as a parameter, of different types of data
// used by the protocol.

char ReadByte(int s)
{
    // Returns 1 byte read from socket, or abort execution on error
    char byte;
    int read;
    read = recv(s, &byte, 1, 0);
    if (read != 1)
    {
        perror("When reading byte");
        exit(1);
    }
    return byte;
}

short int ReadShort(int s)
{
    // Returns 1 short read from the socket, or abort execution on error
    // Wait for the short in network format and convert it to the host's endianity
    short int data;
    int read;
    read = recv(s, &data, 2, 0);
    if (read != 2)
    {
        perror("When reading short");
        exit(1);
    }
    return ntohs(data);
}

void ReadString(int s, char *buff, int len)
{
    // Read a string of given length and drop it at the specified address in s
    // It is the responsibility of the calling program to ensure that in the address
    // pointed there is enough space to hold the received bytes
    int read;
    read = recv(s, buff, len, 0);
    if (read != len)
    {
        perror("When reading string");
        exit(1);
    }
}

void ReadEndpoint(int s, struct sockaddr_in *endpoint)
{
    // Read an IP and port and leave them in the sockaddr_in structure that receives
    // as a parameter. The IP and port are expected in the network format used
    // by TCP IP protocol
    int read;
    read = recv(s, &(endpoint->sin_addr.s_addr), 4, 0);
    if (read != 4)
    {
        perror("When reading ip");
        exit(1);
    }
    read = recv(s, &(endpoint->sin_port), 2, 0);
    if (read != 2)
    {
        perror("When reading port");
        exit(1);
    }
}

void SendByte(int s, char byte)
{
    int sent;
    sent = send(s, &byte, 1, 0);
    if (sent != 1)
    {
        perror("When sending byte");
        exit(1);
    }
}

void SendShort(int s, short int data)
{
    int sent;
    short int data_ok = htons(data);
    sent = send(s, &data_ok, 2, 0);
    if (sent != 2)
    {
        perror("When sending short");
        exit(1);
    }
}

void SendString(int s, char *str)
{
    int sent;
    sent = send(s, str, strlen(str), 0);
    if (sent != strlen(str))
    {
        perror("When sending string");
        exit(1);
    }
}

void SendEndpoint(int s, struct sockaddr_in *endpoint)
{
    int sent;
    sent = send(s, &(endpoint->sin_addr), 4, 0);
    if (sent != 4)
    {
        perror("When sending ip");
        exit(1);
    }
    sent = send(s, &(endpoint->sin_port), 2, 0);
    if (sent != 2)
    {
        perror("When sending port");
        exit(1);
    }
}