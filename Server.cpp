#include "Server.h"

bool addSocket(Server& i_Server, SOCKET id, eSocketStatus what)
{
	unsigned long flag = 1;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (i_Server.sockets[i].recv == eSocketStatus::EMPTY)
		{
			i_Server.sockets[i].id = id;
			i_Server.sockets[i].recv = what;
			i_Server.sockets[i].send = eSocketStatus::IDLE;
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
	i_Server.sockets[index].recv = eSocketStatus::EMPTY;
	i_Server.sockets[index].send = eSocketStatus::EMPTY;
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

	if (addSocket(i_Server, msgSocket, eSocketStatus::RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(Server& i_Server, int index)
{
	SOCKET msgSocket = i_Server.sockets[index].id;
	i_Server.sockets[index].timer = time(0);
	int len = i_Server.sockets[index].len;
	int bytesRecv = recv(msgSocket, &i_Server.sockets[index].buffer[len], sizeof(i_Server.sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Web Server: Error at recv(): " << WSAGetLastError() << endl;
		terminateSocket(msgSocket, i_Server, index);
		return;
	}

	if (bytesRecv == 0)
	{
		terminateSocket(msgSocket, i_Server, index);
		return;
	}
	else
	{
		i_Server.sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Web Server: Recieved: " << bytesRecv << " bytes of \"" << &i_Server.sockets[index].buffer[len] << "\" message.\n";
		i_Server.sockets[index].len += bytesRecv;
		messageHandler(i_Server, index);
	}
}

void messageHandler(Server& i_Server, int index)
{
	SOCKET msgSocket = i_Server.sockets[index].id;
	if (i_Server.sockets[index].len > 0)
	{
		parseResponse(i_Server, index);
		if (i_Server.sockets[index].request.find("Method") != i_Server.sockets[index].request.end())
		{
			i_Server.sockets[index].send = eSocketStatus::SEND;
		}
		//we will never get here
		else if (strncmp(i_Server.sockets[index].buffer, "Exit", 4) == 0)
		{
			closesocket(msgSocket);
			removeSocket(i_Server, index);
			return;
		}
	}
}

void terminateSocket(SOCKET& socket, Server& server, int index)
{
	closesocket(socket);
	removeSocket(server, index);
}

void sendMessage(Server& i_Server, int index)
{
	int bytesSent = 0;
	string sendBuff;
	Response response;
	SOCKET msgSocket = i_Server.sockets[index].id;
	string method = i_Server.sockets[index].request["Method"];
	string continueTo = i_Server.sockets[index].continueTo;
	if (method == "GET")
	{
		response = generateGetResponse(i_Server, index);
	}
	else if (method == "POST")
	{
		response = generatePostResponse(i_Server, index);
	}
	else if (method == "HEAD")
	{
		response = generateHeadResponse(i_Server, index);
	}
	else if (method == "PUT" || continueTo == "PUT")
	{
		response = generatePutResponse(i_Server, index);
	}
	else if (method == "DELETE")
	{
		response = generateDeleteResponse(i_Server, index);
	}
	else if (method == "OPTIONS")
	{
		response = generateOptionsResponse(i_Server, index);
	}
	else if (method == "TRACE")
	{
		response = generateTraceResponse(i_Server, index);
	}
	else
	{
		//method not allowd 
		return;
	}
	sendBuff = convertResponseToString(response);
	bytesSent = send(msgSocket, sendBuff.c_str(), (int)sendBuff.size(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Web Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Web Server: Sent: " << bytesSent << "\\" << sendBuff.size() << " bytes of \"" << sendBuff << "\" message.\n";

	i_Server.sockets[index].send = eSocketStatus::IDLE;
}

void initWinsock()
{
	WSAData wsaData;
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

	addSocket(i_Server, i_Server.listenSocket, eSocketStatus::LISTEN);
	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((i_Server.sockets[i].recv == eSocketStatus::LISTEN)
				|| (i_Server.sockets[i].recv == eSocketStatus::RECEIVE))
			{
				FD_SET(i_Server.sockets[i].id, &waitRecv);
			}
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (i_Server.sockets[i].send == eSocketStatus::SEND)
			{
				FD_SET(i_Server.sockets[i].id, &waitSend);
			}
		}

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
				if (i_Server.sockets[i].recv == eSocketStatus::LISTEN)
				{
					acceptConnection(i_Server, i);
				}
				else if (i_Server.sockets[i].recv == eSocketStatus::RECEIVE)
				{
					receiveMessage(i_Server, i);
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
				case eSocketStatus::SEND:
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
	i_Server.listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);
	if (SOCKET_ERROR == bind(i_Server.listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}

	if (SOCKET_ERROR == listen(i_Server.listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(i_Server.listenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void getSubType(Server& i_Server, int index)
{
	string buffer = i_Server.sockets[index].buffer;
	size_t found = buffer.find('?');
	if (found != string::npos)
	{
		i_Server.sockets[index].isQuary = true;
		i_Server.sockets[index].quary = buffer.substr(found + 6, 2);
	}
}

string htmlToString(ifstream& htmlFile)
{
	string temp;
	string output;
	while (getline(htmlFile, temp))
	{
		output += temp += Response::r_NewLine;
	}

	return output;
}

Response generateGetResponse(Server& i_Server, int index)
{
	getSubType(i_Server, index);
	ifstream htmlFile;
	string htmlPath = i_Server.sockets[index].request["Path"];
	if (htmlPath.empty())
	{
		return Response();
	}

	htmlPath.erase(0, 1);
	string fileAsString;
	Response output;
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

	if (htmlPath == "")
	{
		htmlPath = "index-en.html";
	}

	htmlFile.open(htmlPath);
	if (htmlFile.is_open())
	{
		fileAsString = htmlToString(htmlFile);
		output.m_Code = eCode::OK;
		output.m_ContentLength = fileAsString.size();
		output.m_Body = fileAsString;
		htmlFile.close();
	}
	else
	{
		output.m_Code = eCode::NotFound;
		output.m_ContentLength = 0;
	}

	return output;
}

Response generatePostResponse(Server& i_Server, int index)
{
	Response output = generateGetResponse(i_Server, index);
	string path = "data.txt";
	string body = getBody(i_Server.sockets[index].buffer);
	body += "\n";
	ofstream fileToWrite;
	fileToWrite.open(path, ios::app);
	if (fileToWrite.is_open())
	{
		fileToWrite << body;
		output.m_Code = eCode::OK;
	}
	else
	{
		fileToWrite << body;
		output.m_Code = eCode::Created;
	}
	printBodyParameters(i_Server, index);
	
	return output;
}

Response generateHeadResponse(Server& i_Server, int index)
{
	Response output = generateGetResponse(i_Server, index);
	output.m_Body.clear();
	return output;
}

Response generatePutResponse(Server& i_Server, int index)
{
	string buffer = i_Server.sockets[index].buffer;
	Response output;
	string& continueTo = i_Server.sockets[index].continueTo;
	string& prevPath = i_Server.sockets[index].prevPath;


	if (i_Server.sockets[index].request["Expect"] == "100-continue")
	{
		continueTo = "PUT";
		prevPath = i_Server.sockets[index].request["Path"];
		output.m_Code = eCode::Continue;
	}

	if (isBodyExist(buffer))
	{
		// move body to file
		prevPath.erase(0, 1);
		ofstream fileToWrite;
		fileToWrite.open(prevPath, ios::trunc);
		if (fileToWrite.is_open())
		{
			string body = getBody(i_Server.sockets[index].buffer);
			fileToWrite << body;
			output.m_Code = eCode::Created;
			output.m_Body = body;
			output.m_ContentLength = body.size();
			output.m_ContentLocation = prevPath;
			continueTo.clear();
			fileToWrite.close();
		}
		else
		{
			output.m_Code = eCode::InternalServerError;
		}

	}

	return output;
}

Response generateDeleteResponse(Server& i_Server, int index)
{
	Response output;
	string path = i_Server.sockets[index].request["Path"];
	path.erase(0, 1);
	if (remove(path.c_str()) == 0)
	{
		output.m_Code = eCode::OK;
	}
	else
	{
		output.m_Code = eCode::NotFound;
	}
	return output;
}

Response generateOptionsResponse(Server& i_Server, int index)
{
	Response output = generateGetResponse(i_Server, index);
	output.m_Allow = Response::r_AllowMethods;
	return output;
}

Response generateTraceResponse(Server& i_Server, int index)
{
	Response output;
	string buffer = i_Server.sockets[index].buffer;
	output.m_Body = buffer;
	output.m_ContentLength = buffer.size();
	output.r_ContentType = "message/http";
	output.m_Code = eCode::OK;
	return output;
}

void printBodyParameters(Server& i_Server, int index)
{
	string buffer = i_Server.sockets[index].buffer;
	if (isBodyExist(buffer))
	{
		string body = getBody(buffer);
		size_t pos = body.find('=');
		if (pos != string::npos)
		{
			string entityBody = body.substr(0, pos);;
			string octet = body.substr(pos + 1, body.size());
			cout << "POST Response Body = entity-body: " << entityBody << " *OCTET: " << octet << endl;
		}
	}
}

void parseResponse(Server& i_Server, int index)
{
	string buffer = i_Server.sockets[index].buffer;
	map<string, string>& request = i_Server.sockets[index].request;
	string method = GetSubHeader(buffer, " ", 0);
	mapInsert(request, "Method", method);
	string path = GetSubHeader(buffer, " ", 0);
	mapInsert(request, "Path", path);
	string version = GetSubHeader(buffer, "\r", 2);
	mapInsert(request, "Version", version);

	while (buffer.size() > 1 && buffer[0] != '\r')
	{
		string key = GetSubHeader(buffer, ":", 2);
		if (!key.empty())
		{
			string value = GetSubHeader(buffer, "\r", 2);
			if (!value.empty())
			{
				mapInsert(request, key, value);
			}
		}
	}
}

string GetSubHeader(string& buffer, string lookFor, int offset)
{
	deleteBegingSpaces(buffer);
	size_t pos = buffer.find(lookFor);
	string result;
	if (pos != string::npos)
	{
		result = buffer.substr(0, pos);
		buffer.erase(0, pos + offset);
	}
	return result;
}

void deleteBegingSpaces(string& i_Input)
{
	size_t pos = i_Input.find_first_not_of(" ");
	if (pos != string::npos)
	{
		i_Input = i_Input.substr(pos);
	}
}

void mapInsert(map<string, string>& i_Request, string i_Key, string i_Value)
{
	map<string, string>::iterator it;
	it = i_Request.find(i_Key);
	i_Request[i_Key] = i_Value;
}

bool isBodyExist(string i_buffer)
{
	size_t pos = i_buffer.find("\r\n\r\n");
	return (i_buffer.size() >= pos + 5);
}

string getBody(string i_Buffer)
{
	size_t pos = i_Buffer.find("\r\n\r\n");
	string res;
	if (pos != string::npos)
	{
		res = i_Buffer.substr(pos + 4, i_Buffer.size());
		pos = i_Buffer.find("\r");
		if (pos != string::npos)
		{
			res = res.substr(0, pos);
		}
	}

	return res;
}

void isTimeOut(Server& i_Server, int index)
{
	size_t now = time(0);
	for (int i = 1; i < MAX_SOCKETS; i++)
	{
		if (now > MAXTIMEOUT)
		{

		}
	}
}