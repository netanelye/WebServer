#include "Server.h"

bool addSocket(Server& i_Server, SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (i_Server.sockets[i].recv == EMPTY)
		{
			i_Server.sockets[i].id = id;
			i_Server.sockets[i].recv = what;
			i_Server.sockets[i].send = IDLE;
			i_Server.sockets[i].len = 0;
			i_Server.socketsCount++;
			unsigned long flag = 1;
			if (ioctlsocket(i_Server.sockets[i].id, FIONBIO, &flag) != 0)
			{
				cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			}
			return (true);
		}
	}
	return (false);
}

void removeSocket(Server& i_Server, int index)
{
	i_Server.sockets[index].recv = EMPTY;
	i_Server.sockets[index].send = EMPTY;
	i_Server.socketsCount--;
}

void acceptConnection(Server& i_Server, int index)
{
	SOCKET id = i_Server.sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	//unsigned long flag = 1;
	//if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	//{
	//	cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	//}

	if (addSocket(i_Server, msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(Server& i_Server, int index)
{
	SOCKET msgSocket = i_Server.sockets[index].id;

	int len = i_Server.sockets[index].len;
	int bytesRecv = recv(msgSocket, &i_Server.sockets[index].buffer[len], sizeof(i_Server.sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(i_Server, index);
		return;
	}

	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(i_Server, index);
		return;
	}
	else
	{
		i_Server.sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &i_Server.sockets[index].buffer[len] << "\" message.\n";

		i_Server.sockets[index].len += bytesRecv;

		if (i_Server.sockets[index].len > 0)
		{
			if (strncmp(i_Server.sockets[index].buffer, "TimeString", 10) == 0)
			{
				i_Server.sockets[index].send = SEND;
				i_Server.sockets[index].sendSubType = SEND_TIME;
				memcpy(i_Server.sockets[index].buffer, &i_Server.sockets[index].buffer[10], i_Server.sockets[index].len - 10);
				i_Server.sockets[index].len -= 10;
				return;
			}
			else if (strncmp(i_Server.sockets[index].buffer, "SecondsSince1970", 16) == 0)
			{
				i_Server.sockets[index].send = SEND;
				i_Server.sockets[index].sendSubType = SEND_SECONDS;
				memcpy(i_Server.sockets[index].buffer, &i_Server.sockets[index].buffer[16], i_Server.sockets[index].len - 16);
				i_Server.sockets[index].len -= 16;
				return;
			}

			else if (strncmp(i_Server.sockets[index].buffer, "Exit", 4) == 0)
			{
				closesocket(msgSocket);
				removeSocket(i_Server, index);
				return;
			}
		}
	}
}

void sendMessage(Server& i_Server, int index)
{
	int bytesSent = 0;
	char sendBuff[255];

	SOCKET msgSocket = i_Server.sockets[index].id;
	if (i_Server.sockets[index].sendSubType == SEND_TIME)
	{
		// Answer client's request by the current time string.

		// Get the current time.
		time_t timer;
		time(&timer);
		// Parse the current time to printable string.
		strcpy(sendBuff, ctime(&timer));
		sendBuff[strlen(sendBuff) - 1] = 0; //to remove the new-line from the created string
	}
	else if (i_Server.sockets[index].sendSubType == SEND_SECONDS)
	{
		// Answer client's request by the current time in seconds.

		// Get the current time.
		time_t timer;
		time(&timer);
		// Convert the number to string.
		_itoa((int)timer, sendBuff, 10);
	}

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	i_Server.sockets[index].send = IDLE;
}

void initWinsock()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}
}

void run(Server& i_Server)
{
	initWinsock();

	if (!initListenSocket(i_Server))
	{
		return;
	}

	if (!initServerSide(i_Server))
	{
		return;
	}

	addSocket(i_Server, i_Server.listenSocket, LISTEN);

	// Accept connections and handles them one by one.
	while (true)
	{
		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((i_Server.sockets[i].recv == LISTEN) || (i_Server.sockets[i].recv == RECEIVE))
			{
				FD_SET(i_Server.sockets[i].id, &waitRecv);
			}	
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (i_Server.sockets[i].send == SEND)
			{
				FD_SET(i_Server.sockets[i].id, &waitSend);
			}	
		}


		// Wait for interesting event.
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		//
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(i_Server.sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (i_Server.sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i_Server, i);
					break;

				case RECEIVE:
					receiveMessage(i_Server, i);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(i_Server.sockets[i].id, &waitSend))
			{
				nfd--;
				switch (i_Server.sockets[i].send)
				{
				case SEND:
					sendMessage(i_Server, i);
					break;
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(i_Server.listenSocket);
	WSACleanup();
}

bool initListenSocket(Server& i_Server)
{
	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
	i_Server.listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == i_Server.listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return false;
	}
	return true;
}

bool initServerSide(Server& i_Server)
{

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(i_Server.listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(i_Server.listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}
