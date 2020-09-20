#include "Response.h"

const string Response::r_NewLine = "\r\n";
const string Response::r_AllowMethods = "GET, HEAD, PUT, OPTIONS, DELETE, TRACE, POST";

string convertResponseToString(Response& i_Response)
{
	time_t timer;
	time(&timer);
	string date = ctime(&timer);
	date.pop_back();

	string output = i_Response.r_HTTPVersion += " ";
	output += to_string((int)i_Response.m_Code) + " ";
	output += eCodeToString(i_Response.m_Code) + Response::r_NewLine;
	output += string("Date: ") + date + Response::r_NewLine;
	output += string("Cache-Control: ") + i_Response.m_CacheControl + Response::r_NewLine;
	output += string("Content-Type: ") + i_Response.r_ContentType + Response::r_NewLine;
	if (!i_Response.m_ContentLocation.empty())
	{
		output += string("Content-Location: ") + i_Response.m_ContentLocation + Response::r_NewLine;
	}
	if (i_Response.m_ContentLength >= 0)
	{
		output += string("Content-Length: ") + to_string(i_Response.m_ContentLength) + Response::r_NewLine;
	}
	if (!i_Response.m_Allow.empty())
	{
		output += string("Allow: ") + i_Response.m_Allow + Response::r_NewLine;
	}
	output += Response::r_NewLine;
	if (!i_Response.m_Body.empty())
	{
		output +=i_Response.m_Body + Response::r_NewLine;
	}
	return output;
}

string eCodeToString(eCode i_Code)
{
	string output;
	switch (i_Code)
	{
	case eCode::Continue:
		output = "Continue";
		break;
	case eCode::OK:
		output = "OK";
		break;
	case eCode::NotFound:
		output = "Not Found";
		break;
	case eCode::Created:
		output = "Created";
		break;
	case eCode::Accepted:
		output = "Accepted";
		break;
	
	case eCode::InternalServerError:
		output = "Internal Server Error";
		break;
	}
	return output;
}