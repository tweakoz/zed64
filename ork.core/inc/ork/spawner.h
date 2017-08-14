///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/environment.h>
 #include <sys/wait.h>

namespace ork{
///////////////////////////////////////////////////////////////////////////////

//! process spawner abstractor
struct Spawner
{
	Spawner();
	~Spawner();

	void spawn();
	bool is_alive();
    void sendSignal (int sig);
    void collectZombie ();
	
	Environment mEnvironment;
	std::string mWorkingDirectory;
	std::string mCommandLine;
	pid_t mChildPID;
	int mExecRet;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace ork
