#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <string>
#include <time.h>
#include "eCode.h"
using namespace std;

struct Response
{
	string r_HTTPVersion = "HTTP/1.1";
	eCode m_Code = eCode::OK;
	size_t m_ContentLength = 0;
	string r_ContentType = "text/html";
	string m_Body;
	string m_Allow;
	string m_CacheControl = "no-cache, private";
	string m_ContentLocation;
	static const string r_NewLine;
	static const string r_AllowMethods;
};

string convertResponseToString(Response& i_Response);
string eCodeToString(eCode i_Code);
