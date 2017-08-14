///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>

namespace ork {
///////////////////////////////////////////////////////////////////////////////

//! Process Environment variable handling class
class Environment
{
public:
	typedef std::map<std::string,std::string> env_map_t;

	Environment();

	void init_from_envp(char** envp);
	void set( const std::string& k, const std::string& v );
	bool has( const std::string& k ) const;
	bool get( const std::string& k, std::string& vout ) const;

	void dump() const;
	
	const env_map_t& RefMap() const { return mEnvMap; }

private:

	env_map_t mEnvMap;

};

///////////////////////////////////////////////////////////////////////////////
} // namespace ork
