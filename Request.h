#pragma once
#include <iostream>
#include <map>
#include "eCode.h"
using namespace std;

enum eMethod {Head, Get, Options,Post,Put,Delete, Trace};

struct Request
{
	eMethod m_Method;
	string m_Path;
	string r_HTTPVersion = "HTTP/1.1";
	int m_ContentLength = 0;
	string r_contentType = "text/html";
	time_t m_Date;
	string m_Body;
};
