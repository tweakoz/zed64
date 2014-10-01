import os
import imp
import ConfigParser

__all__ = [ "ISE_DIR", "ISE_BIN_DIR" ]

################################################################

def GetDefault( varname, default ):
	ret = default
	if varname in os.environ:
		ret = os.environ[varname]
	if False==os.path.isdir(ret):
		print "<localopts.py> Warning: path<%s> <ret %s> does not exist" % (varname,ret)
	return os.path.normpath(ret)

################################################################

def ConfigFileName():
	return "./zed.build.ini"

ConfigData = ConfigParser.ConfigParser()

ise_dir = "/opt/Xilinx/14.7/ISE_DS/ISE"
ise_bindir = "%s/bin/lin64" % ise_dir

if os.path.isfile( ConfigFileName() ):
  ConfigData.read( ConfigFileName() )
else:
 print "LOCALOPTS: Cannot find %s : using default options" % ConfigFileName()
 ConfigData.add_section( "PATHS" )
 ConfigData.set( "PATHS", "ISE_DIR", GetDefault("ISE_DIR", ise_dir) )
 ConfigData.set( "PATHS", "ISE_BIN_DIR", GetDefault("ISE_BIN_DIR", ise_bindir ) )
 cfgfile = open(ConfigFileName(),'w')
 ConfigData.write(cfgfile)
 cfgfile.close()

################################################################

def GetEnv( sect, varname ):
    ret = ""
    if ConfigData.has_option( sect, varname ):
        ret = ConfigData.get( sect, varname )
    if os.path.isdir(ret):
		ret = ret
    return os.path.normpath(ret)

################################################################

def ISE_BIN_DIR():
 return GetEnv( "PATHS", "ISE_BIN_DIR" )

