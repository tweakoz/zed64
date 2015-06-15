///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/path.h>
#include <string.h>
#include <stdarg.h>
#include <string>
#include <sstream>
#include <vector>

namespace ork {

///////////////////////////////////////////////////////////////////////////////

std::string FormatString( const char* formatstring, ... )
{
	std::string rval;///////////////////////////////////////////////////////////////////////////////

	char formatbuffer[512];

	va_list args;
	va_start(args, formatstring);
	//buffer.vformat(formatstring, args);
#if 1 //defined(IX)
    vsnprintf( &formatbuffer[0], sizeof(formatbuffer), formatstring, args );
#else
	vsnprintf_s( &formatbuffer[0], sizeof(formatbuffer), sizeof(formatbuffer), formatstring, args );
#endif
	va_end(args);
	rval = formatbuffer;
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

void SplitString(const std::string& s, char delim, std::vector<std::string>& tokens)
{	std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
        tokens.push_back(item);
}

///////////////////////////////////////////////////////////////////////////////

std::vector<std::string> SplitString(const std::string& instr, char delim)
{	std::vector<std::string> tokens;
    SplitString(instr, delim, tokens);
    return tokens;
}

///////////////////////////////////////////////////////////////////////////////

bool IsSubStringPresent(const std::string& needle, const std::string& haystack)
{
	return haystack.find(needle)!=std::string::npos;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ork