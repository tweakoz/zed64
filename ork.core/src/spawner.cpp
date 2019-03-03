///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/spawner.h>
#include <ork/stringutils.h>
#include <assert.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <stdlib.h>

namespace ork {
///////////////////////////////////////////////////////////////////////////////
// process spawn utils
///////////////////////////////////////////////////////////////////////////////

Spawner::Spawner()
    : mChildPID(-1)
    , mExecRet(0)
{

}

///////////////////////////////////////////////////////////////////////////////

Spawner::~Spawner()
{
    if( mChildPID > 0 )
    {
        printf( "KILLING PID<%d>\n", mChildPID );
        sendSignal(SIGKILL);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Spawner::sendSignal (int sig) {
    kill(mChildPID, sig);
}

void Spawner::spawn()
{
    mChildPID = fork();

    //printf( "fork<%d>\n", mChildPID );
    fflush(stdout);

    if( 0 == mChildPID ) // child
    {
        /////////////////////////////////////////////////////////////
        // build environ
        /////////////////////////////////////////////////////////////

        const Environment::env_map_t& emap = mEnvironment.RefMap();

        size_t inum_vars = emap.size();

        char** env_vars = (char**) malloc(sizeof(char*)*(inum_vars+1));

        size_t icounter = 0;
        for( const auto& item : emap )
        {
            const std::string& k = item.first;
            const std::string& v = item.second;
            std::string VAR = k + "=" + v;
            env_vars[icounter] = strdup(VAR.c_str());
            //printf( "SETENV<%s>\n", env_vars[icounter] );
            icounter++;
        }
        env_vars[icounter] = 0; // terminate envvar array

        //printf( "child cp0 numenvvars<%d>\n", int(inum_vars) );
        //fflush(stdout);

        /////////////////////////////////////////////////////////////
        // build args
        /////////////////////////////////////////////////////////////

        std::vector<std::string> vargs = SplitString(mCommandLine,' ');

        //vargs.insert(vargs.begin(),vargs[0]);

        size_t inum_args = vargs.size();

        printf( "child cp1 numargs<%d>\n", int(inum_args) );
        fflush(stdout);

        char** args =  (char**) malloc(sizeof(char*)*(inum_args+1));

        for( int i=0; i<inum_args; i++ )
        {
            const std::string& arg = vargs[i];

            printf( "arg<%d> <%s>\n", i, arg.c_str() );

            if( 0 == i )
            {
                std::string targ = mWorkingDirectory + "" + arg;
                args[i] = strdup(targ.c_str());
                args[i] = strdup(targ.c_str());
            }
            else
            {
                args[i] = strdup(arg.c_str());
            }
            
            //kernel::glog.printf( "spawn arg<%d:%s>\n", i, args[i] );
        }
        args[inum_args] = 0; // terminate arg array

        //printf( "child cp2\n" );
        //fflush(stdout);

        /////////////////////////////////////////////////////////////
        // set cwd
        /////////////////////////////////////////////////////////////

        if( mWorkingDirectory.length() )
        {
            printf( "child changing to directory<%s>\n", mWorkingDirectory.c_str() );
            int iret = chdir( mWorkingDirectory.c_str() );
            assert(iret==0);
        }

        /////////////////////////////////////////////////////////////
        // exec
        /////////////////////////////////////////////////////////////

        printf( "child calling exec exe<%s>\n", args[0] );

        mExecRet = execve( args[0], args, env_vars );

       // kernel::glog.printf( "fork failed <child> mExecRet<%d> ERRNO<%d>\n", mExecRet, errno );

        perror("FORK FAILED");

        assert(false); // if exec fails, exit forked process
    }
    else if( mChildPID<0 )
    {
        assert(false); // failed to fork
    }
    else // parent
    {
        //printf( "Spawned child pid<%d>\n", mChildPID );
    }

}

bool Spawner::is_alive()
{
    int status;
    int err = waitpid(mChildPID, &status, WNOHANG);

    if (-1 == err) {
        printf("Spawner<%p>::is_alive: waitpid: %s\n", this, strerror(errno));
        return false;
    }

    return 0 == err;
}

/** Block until the child changes state. */
void Spawner::collectZombie () {
    int status;
    int err = waitpid(mChildPID, &status, 0);

    if (-1 == err) {
        printf("Spawner<%p>::collectZombie: waitpid: %s\n", this, strerror(errno));
    }
}
///////////////////////////////////////////////////////////////////////////////
} // namespace ork
