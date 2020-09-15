#pragma once
#include <iostream>
#include <string>

using namespace std;


enum eCode { Continue = 100, OK = 200, Created = 201, Accepted = 202, NotFound = 404};
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
	string cacheControl = "no-cache, private";// delete
	static string newLine;
};



string convertResponseToString(Response& i_Response);
string eCodeToString(eCode i_Code);
