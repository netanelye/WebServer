#pragma once
#include <iostream>
#include <map>
using namespace std;
enum eCode { Continue = 100, OK = 200, Created = 201, Accepted = 202, NotFound = 404 };
enum eMethod {Head, Get, Options,Post,Put,Delete, Trace};
typedef struct request
{
	eMethod m_Method;
	string m_Path;
	string HTTPVersion = "HTTP/1.1";
	int contentLength = 0;
	string m_ContentType = "text/html";
	time_t m_Date;
	string body;
};


void parseRequest(;