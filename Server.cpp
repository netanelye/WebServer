#include "Server.h"

bool addSocket(Server& i_Server, SOCKET id, int what)
{
	unsigned long flag = 1;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (i_Server.sockets[i].recv == EMPTY)
		{
			i_Server.sockets[i].id = id;
			i_Server.sockets[i].recv = what;
			i_Server.sockets[i].send = IDLE;
			i_Server.sockets[i].len = 0;
			i_Server.socketsCount++;
			if (ioctlsocket(i_Server.sockets[i].id, FIONBIO, &flag) != 0)
			{
				cout << "Web Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			}
			return true;
		}
	}
	return false;
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
		cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

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
		cout << "Web Server: Error at recv(): " << WSAGetLastError() << endl;
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
		cout << "Web Server: Recieved: " << bytesRecv << " bytes of \"" << &i_Server.sockets[index].buffer[len] << "\" message.\n";

		i_Server.sockets[index].len += bytesRecv;

		if (i_Server.sockets[index].len > 0)
		{
			if (strncmp(i_Server.sockets[index].buffer, "GET", 3) == 0)
			{
				getSubType(i_Server, index);
				i_Server.sockets[index].send = SEND;
				i_Server.sockets[index].sendSubType = SEND_GET;
				memcpy(i_Server.sockets[index].buffer, &i_Server.sockets[index].buffer[3], i_Server.sockets[index].len - 3);
				i_Server.sockets[index].len -= 3;
				return;
			}
			if (strncmp(i_Server.sockets[index].buffer, "POST", 4) == 0)
			{
				i_Server.sockets[index].send = SEND;
				i_Server.sockets[index].sendSubType = SEND_POST;
				memcpy(i_Server.sockets[index].buffer, &i_Server.sockets[index].buffer[4], i_Server.sockets[index].len - 4);
				i_Server.sockets[index].len -= 4;
				return;
			}
			if (strncmp(i_Server.sockets[index].buffer, "HEAD", 4) == 0)
			{
				i_Server.sockets[index].send = SEND;
				i_Server.sockets[index].sendSubType = SEND_HEAD;
				memcpy(i_Server.sockets[index].buffer, &i_Server.sockets[index].buffer[4], i_Server.sockets[index].len - 4);
				i_Server.sockets[index].len -= 4;
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
	string sendBuff;
	Response response;
	SOCKET msgSocket = i_Server.sockets[index].id;
	if (i_Server.sockets[index].sendSubType == SEND_GET)
	{
		response = get(i_Server, index);
	}
	else if (i_Server.sockets[index].sendSubType == SEND_POST)
	{
		response = post(i_Server, index);
	}
	else if (i_Server.sockets[index].sendSubType == SEND_HEAD)
	{
		response = head(i_Server, index);
	}
	sendBuff = convertResponseToString(response);
	bytesSent = send(msgSocket, sendBuff.c_str(), sendBuff.size(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Web Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Web Server: Sent: " << bytesSent << "\\" << sendBuff.size() << " bytes of \"" << sendBuff << "\" message.\n";

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
		cout << "Web Server: Error at WSAStartup()\n";
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
			cout << "Web Server: Error at select(): " << WSAGetLastError() << endl;
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
	cout << "Web Server: Closing Connection.\n";
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
		cout << "Web Server: Error at socket(): " << WSAGetLastError() << endl;
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
	serverService.sin_addr.s_addr = INADDR_ANY;//inet_addr("10.100.102.12")
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
		cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(i_Server.listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void initServer(Server& i_Server)
{

}

void initRequests(Server& i_Server)
{
	const int SEND_OPTIONS = 1;
	const int SEND_GET = 2;
	const int SEND_HEAD = 3;
	const int SEND_POST = 4;
	const int SEND_PUT = 5;
	const int SEND_DELETE = 6;
	const int SEND_TRACE = 7;

	i_Server.requests[0].reqAsString = "GET";
	i_Server.requests[1].reqAsString = "POST";
	i_Server.requests[2].reqAsString = "GET";
	i_Server.requests[3].reqAsString = "GET";
	i_Server.requests[4].reqAsString = "GET";
	i_Server.requests[5].reqAsString = "GET";
	i_Server.requests[6].reqAsString = "GET";
}

void getSubType(Server& i_Server, int index)
{
	string buffer = i_Server.sockets[index].buffer;
	int found = buffer.find('?');
	if (found != string::npos)
	{
		i_Server.sockets[index].isQuary = true;
		i_Server.sockets[index].quary = buffer.substr(found + 6, 2);
	}
}

Response get(Server& i_Server, int index)
{
	ifstream htmlFile;
	string htmlPath = getPathFromGetReq(string(i_Server.sockets[index].buffer));
	string fileAsString;
	Response outPut;
	if (i_Server.sockets[index].isQuary)
	{
		if (i_Server.sockets[index].quary == "he")
		{
			htmlPath = "index-he.html";
		}
		else if (i_Server.sockets[index].quary == "en")
		{
			htmlPath = "index-en.html";
		}
	}

	if (htmlPath == " ")
	{
		htmlPath = "index-en.html";
	}

	htmlFile.open(htmlPath);
	if (htmlFile.is_open())
	{
		fileAsString = htmlToString(htmlFile);
		outPut.code = ok;
		outPut.cacheControl = "no-cache, private";
		outPut.contentLength = fileAsString.size();
		outPut.body = fileAsString;
	}
	else
	{
		outPut.code = notFound;
		outPut.contentLength = 0;
	}

	return outPut;
}

string htmlToString(ifstream& htmlFile)
{
	string temp;
	string output;
	while (getline(htmlFile, temp)) 
	{
		output += temp;
	}

	return output;
}

string getPathFromGetReq(string i_Request)
{
	string path;
	size_t posSlash = i_Request.find('/');
	if (posSlash != string::npos)
	{
		path = i_Request.substr(posSlash);
		size_t posSpace = path.find(' ');
		if (posSpace != string::npos)
		{
			path = path.substr(posSlash, posSpace);
		}
		else
		{
			path = "";
		}
	}
	
	return path;
}

Response post(Server& i_Server, int index)
{
	Response output;
	string x = i_Server.sockets[index].buffer;
	string name = "yourName";
	size_t namePos = x.find(name);
	if (namePos != string::npos)
	{
		x = x.substr(namePos);
		size_t posEqual = x.find('=');
		if (posEqual != string::npos)
		{
			x = x.substr(posEqual +1);
		}
	}
	return output;
}

Response head(Server& i_Server, int index)
{
	Response output = get(i_Server, index);
	output.body = "";
	return output;
}
