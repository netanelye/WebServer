#include "Response.h"

string Response::newLine = "\r\n";

string convertResponseToString(Response& i_Response)
{
	string output = i_Response.HTTPVersion += " ";
	output += to_string((int)i_Response.code) + " ";
	output += eCodeToString(i_Response.code) + Response::newLine;
	output += string("Cache-Control: ") + i_Response.cacheControl + Response::newLine;
	output += string("Content-Type: ") + i_Response.contentType + Response::newLine;
	output += string("Content-Length: ") + to_string(i_Response.contentLength) + Response::newLine;
	output  += Response::newLine + i_Response.body + Response::newLine;

	return output;
}

string eCodeToString(eCode i_Code)
{
	string output;
	switch (i_Code)
	{
	case ok:
		output = "OK";
		break;
	case notFound:
		output = "Not Found";
	}
	return output;
}