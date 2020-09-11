#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream> 
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>

typedef struct socketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[128];
	int len;
}SocketState;
const int TIME_PORT = 80;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int GET = 5;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;
typedef struct server
{
	SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;
	SOCKET listenSocket;
}Server;




bool addSocket(Server& i_Server, SOCKET id, int what);
void removeSocket(Server& i_Server, int index);
void acceptConnection(Server& i_Server, int index);
void receiveMessage(Server& i_Server, int index);
void sendMessage(Server& i_Server, int index);

void run(Server& i_Server);
bool initListenSocket(Server& i_Server);
bool initServerSide(Server& i_Server);