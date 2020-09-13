#pragma once
#include <iostream>
#include <string>
using namespace std;


enum eCode { ok = 200, notFound = 404};
struct Response
{
	string HTTPVersion = "HTTP/1.1";
	eCode code;
	//bool shouldClose = false;
	//string contentLocation;
	//string lastModified = "";
	//string serverName = "Apachi";
	int contentLength = 0;
	string contentType = "text/html";
	string body;
	//string allow = "";
	string cacheControl;
	static string newLine;
};

string convertResponseToString(Response& i_Response);
string eCodeToString(eCode i_Code);
