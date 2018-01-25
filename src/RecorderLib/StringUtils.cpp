#include "StdAfx.h"
#include "StringUtils.h"
#include <Shlwapi.h>
#include "defines.h"

using namespace std;

// breaks apart a string into substrings separated by a character string
// does not use a strtok() style list of separator characters
// returns a list of strings
list<string> StringUtils::Explode(const string &inString, const string &separator)
{
	list<string> returnVector;
	string::size_type start = 0;
	string::size_type end = 0;

	while ((end = inString.find (separator, start)) != string::npos)
	{
		returnVector.push_back (inString.substr (start, end-start));
		start = end + separator.size();
	}

	returnVector.push_back (inString.substr (start));
	return returnVector;
}

vector<string> StringUtils::VExplode(const string &inString, const string &separator)
{
	vector<string> returnVector;
	string::size_type start = 0;
	string::size_type end = 0;

	while ((end = inString.find (separator, start)) != string::npos)
	{
		returnVector.push_back (inString.substr (start, end-start));
		start = end + separator.size();
	}

	returnVector.push_back (inString.substr (start));
	return returnVector;
}


string StringUtils::Trim(const string &inString)
{
	string str = inString;

	// trim trailing spaces
	size_t endpos = str.find_last_not_of(" \t\r\n");
	if( string::npos != endpos )
	{
		str = str.substr( 0, endpos+1 );
	}

	// trim leading spaces
	size_t startpos = str.find_first_not_of(" \t\r\n");
	if( string::npos != startpos )
	{
		str = str.substr( startpos );
	}

	return str;
}

string StringUtils::ToLowerCase(const string &inString)
{
	string str;
	int l = (int)inString.length();
	for(int i=0;i<l;i++) str += tolower(inString[i]);
	return str;
}

string StringUtils::ToUpperCase(const string &inString)
{
	string str;
	int l = (int)inString.length();
	for(int i=0;i<l;i++) str += toupper(inString[i]);
	return str;
}

int StringUtils::StringToInt(const std::string &inString)
{
	return ::StrToIntA(inString.c_str());
}

std::string StringUtils::IntToString(int n)
{
	char aa[25];
	SAFE_SNPRINTF(aa, sizeof(aa), "%i", n);
	return string(aa);
}

std::string StringUtils::IntToHex(int n, int len/*=0*/)
{
	char aa[25];
	char format[25];

	if (len)
	{
		SAFE_SNPRINTF(format, sizeof(format), "%%0%dx", len);
		SAFE_SNPRINTF(aa, sizeof(aa), format, n);
	}
	else
	{
		SAFE_SNPRINTF(aa, sizeof(aa), "%x", n);
	}
	return string(aa);
}

std::string StringUtils::PtrToHex(void *p)
{
	char aa[25];
	SAFE_SNPRINTF(aa, sizeof(aa), "%p", p);
	return string(aa);
}

bool StringUtils::IsAllDigits(const std::string &sIn)
{
	int l = sIn.length();
	for (int i = 0; i < l; i++)
	{
		if ((sIn[i] < 0x30) || (sIn[i] > 0x39))
			return false;
	}
	return true;
}


