///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/environment.h>
#include <assert.h>
#include <string.h>

namespace ork {
///////////////////////////////////////////////////////////////////////////////
// environment variable utils
///////////////////////////////////////////////////////////////////////////////

Environment::Environment()
{

}

///////////////////////////////////////////////////////////////////////////////

void Environment::init_from_envp(char** envp)
{
    for( char** env=envp; *env!=0; env++ )
    {
       char* this_env = *env;

        if( this_env )
        {
            std::string estr(this_env);
            const char* pbeg = estr.c_str();
            const char* peq = strstr(pbeg,"=");
            assert(peq[0]=='=');
            size_t klen = peq-pbeg;
            std::string key = estr.substr(0,klen);
            std::string val = estr.substr(klen+1,estr.length());
            mEnvMap[key] = val;
            //printf( "split<%s> k<%s> v<%s>\n", peq, key.c_str(), val.c_str() );
        }
    }

}

///////////////////////////////////////////////////////////////////////////////

void Environment::set( const std::string& k, const std::string& v )
{
    mEnvMap[k] = v;
}

///////////////////////////////////////////////////////////////////////////////

bool Environment::has( const std::string& k ) const
{
    auto it = mEnvMap.find(k);
    return it!=mEnvMap.end();
}

///////////////////////////////////////////////////////////////////////////////

bool Environment::get( const std::string& k, std::string& vout ) const
{
    auto it = mEnvMap.find(k);
    bool brval = it!=mEnvMap.end();
    if( brval )
        vout = it->second;
    return brval;

}

///////////////////////////////////////////////////////////////////////////////

void Environment::dump() const
{
    for( const auto& item : mEnvMap )
    {
        printf( "KEY<%s> VAL<%s>\n", item.first.c_str(), item.second.c_str() );
    }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace ork
