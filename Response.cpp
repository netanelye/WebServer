#include "Response.h"

string Response::newLine = "\r\n";

string convertResponseToString(Response& i_Response)
{
	time_t timer;
	time(&timer);
	string date = ctime(&timer);
	date.pop_back();
	
	string output = i_Response.HTTPVersion += " ";
	output += to_string((int)i_Response.code) + " ";
	output += eCodeToString(i_Response.code) + Response::newLine;
	output += string("Date: ") + date + Response::newLine;
	output += string("Cache-Control: ") + i_Response.cacheControl + Response::newLine;
	output += string("Content-Type: ") + i_Response.contentType + Response::newLine;
	output += string("Content-Length: ") + to_string(i_Response.contentLength) + Response::newLine;
	if (!i_Response.allow.empty())
	{
		output += string("Allow: ") + i_Response.allow + Response::newLine;
	}
	if (!i_Response.body.empty())
	{
		output += Response::newLine + i_Response.body + Response::newLine;
	}

	return output;
}

string eCodeToString(eCode i_Code)
{
	string output;
	switch (i_Code)
	{
	case Continue:
		output = "Continue";
		break;
	case OK:
		output = "OK";
		break;
	case NotFound:
		output = "Not Found";
		break;
	case Created:
		output = "Created";
		break;
	case Accepted:
		output = "Accepted";
		break;
	}
	return output;
}