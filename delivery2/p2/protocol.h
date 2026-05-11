/*
MECHANISM: P2P
FILE: protocol.h
DESCRIPTION: protocol.c interface. It also includes the codes for the protocol
*/

// ADDED Nigel Xherimeja
#ifndef PROTOCOL_H
#define PROTOCOL_H
#define PONG_CMD 0x04
#define PONG_ERR 0x84


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define JOIN_CMD 0x01
#define LEAVE_CMD 0x02
#define QUERY_CMD 0x03
#define JOIN_ERR 0x81
#define LEAVE_ERR 0x82
#define QUERY_ERR 0x83
#define CMD_ERR 0x80
#define CMD_OK 0x00

int Close(int socket);
int CreateTCPDataSocket();
int ListeningSocket(int port);
int Accept(int listening_socket);
int Connect(int sock, char *ip, int port);
char ReadByte(int s);
short int ReadShort(int s);
void ReadString(int s, char *buff, int len);
void ReadEndpoint(int s, struct sockaddr_in *endpoint);

void SendByte(int s, char byte);
void SendShort(int s, short int data);
void SendString(int s, char *buff);
void SendEndpoint(int s, struct sockaddr_in *endpoint);

#endif // ADDED Nigel Xherimeja