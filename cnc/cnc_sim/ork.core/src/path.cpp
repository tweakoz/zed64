///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/path.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ork {

PathMarkers::PathMarkers()
    : mDriveLen(0)
    , mProtocolBaseLen(0)
    , mHostNameLen(0)
    , mPortLen(0)
    , mFolderLen(0)
    , mFileNameLen(0)
    , mExtensionLen(0)
    , mQueryStringLen(0)
{
}

///////////////////////////////////////////////////////////////////////////////
// TODO : bring UnitTest++ and some orkid unit tests into u-ork
//  add unit tests for common http type paths
///////////////////////////////////////////////////////////////////////////////
//
// yo                                   file
// yo.txt                               file ext
// yo.txt/                              folder
// data://yo/dude?dude=yo               url folder file query
// data://yo                            url file
// data://yo/                           url folder
// data://yo.ext                        url file
// data://yo.ext/                       url folder
// data://yo/dude                       url folder file
// data://yo?dude=yo                    url file query
//
///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetDriveBase() const
{
	return unsigned(0);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetProtocolBase() const
{
	return unsigned(0);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetHostNameBase() const
{
    OrkAssert(false); // not implemented
    return unsigned(0);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetPortBase() const
{
    OrkAssert(false); // not implemented
    return unsigned(0);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetFolderBase() const
{
	OrkAssert( false == ((mDriveLen>0)&&(mProtocolBaseLen>0)) );
	unsigned int ib = (mDriveLen>mProtocolBaseLen) ? mDriveLen : mProtocolBaseLen;
	return ib;
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetFileNameBase() const
{
	return GetFolderBase()+mFolderLen;
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetExtensionBase() const
{
	bool bdot = (mExtensionLen>0);

	return GetFileNameBase() + (bdot ? mFileNameLen+1 : mFileNameLen);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int PathMarkers::GetQueryStringBase() const
{
	unsigned iebas = GetExtensionBase();
	unsigned ielen = mExtensionLen;
	return (ielen>0) ? iebas+ielen+1 // extension base + extension length +  ?
					 : iebas+1;      // extension base + ?
}

///////////////////////////////////////////////////////////////////////////////

Path::Path()
	: mPathString("")
	, mMarkers()
{

}

///////////////////////////////////////////////////////////////////////////////

Path::Path(const char* pathName)
	: mPathString("")
	, mMarkers()
{
	Set(pathName);
}

///////////////////////////////////////////////////////////////////////////////

Path::Path(const std::string& pathName)
    : mPathString("")
    , mMarkers()
{
    Set(pathName.c_str());
}

///////////////////////////////////////////////////////////////////////////////

Path::Path(const NameType& pathName)
	: mPathString("")
	, mMarkers()
{
    Set(pathName.c_str());
}

///////////////////////////////////////////////////////////////////////////////

Path::~Path()
{

}

///////////////////////////////////////////////////////////////////////////////

/*Path::HashType Path::Hash() const
{
	NameType copy = mPathString;
	//////////////////
	// 1st pass hash
	U32 uval = CCRC::HashMemory( copy.c_str(), int(strlen(copy.c_str())));
	//orkprintf( "HashPath path<%s> hash<%08x>\n", copy.c_str(), uval );
	//////////////////
	return file::Path::HashType(uval);
}*/

///////////////////////////////////////////////////////////////////////////////

size_t Path::length() const
{
	return mPathString.length();
}

///////////////////////////////////////////////////////////////////////////////

bool Path::empty() const
{
	return mPathString.empty();
}

///////////////////////////////////////////////////////////////////////////////

Path Path::operator+( const Path& oth ) const
{
	Path rval(*this);
	rval.mPathString += oth.mPathString;
	rval.ComputeMarkers('/');
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

void Path::operator=( const Path& oth )
{
	mPathString = oth.mPathString;
	mMarkers = oth.mMarkers;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::operator<( const Path& oth ) const
{
	return( strcmp( c_str(), oth.c_str() ) > 0 );
}

///////////////////////////////////////////////////////////////////////////////

void Path::operator+=( const Path& oth )
{
	mPathString += oth.mPathString;
	ComputeMarkers('/');
}

///////////////////////////////////////////////////////////////////////////////

bool Path::operator!=( const Path& oth ) const
{
	return ( 0!=strcmp(c_str(),oth.c_str()) );
}

///////////////////////////////////////////////////////////////////////////////

bool Path::operator==( const Path& oth ) const
{
	return ( 0==strcmp(c_str(),oth.c_str()) );
}

///////////////////////////////////////////////////////////////////////////////

void Path::SetDrive(const char* Drv)
{
    DecomposedPath decomposed;
	DeCompose( decomposed );

	if( strlen(Drv)==0 )
	{
		if( decomposed.mProtocol.length() != 0 )
		{
			decomposed.mProtocol.set("");
		}
	}
	decomposed.mDrive = Drv;
	Compose( decomposed );
}

///////////////////////////////////////////////////////////////////////////////

void Path::SetUrlBase(const char* newurl)
{
    DecomposedPath decomposed;
    DeCompose( decomposed );
	decomposed.mProtocol = newurl;
    Compose( decomposed );
}

///////////////////////////////////////////////////////////////////////////////

static bool PathPred( const char* src, const char* loc, size_t isrclen )
{   if( (loc-src)>1 )
	{  if( strncmp( loc-1, "://", 3 ) == 0 )
		{ return false;
		}
	}
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void Path::Set( const char* instr )
{
	NameType tmp,tmp2;
	tmp.set( instr );
	//////////////////////////////////////////////
	// convert pathseps to internal format (posix)
	size_t ilen = tmp.length();
	const char* begin = tmp.c_str();
	dos2unixpathsep xform;
	for( size_t i=0; i<ilen; i++ )
	{
		tmp.SetChar( i, xform( begin[i] ) );
	}
	//////////////////////////////////////////////
	tmp2.replace( tmp.c_str(), "/./", "/" );
	mPathString.replace( tmp2.c_str(), "//", "/",  PathPred );
	//////////////////////////////////////////////
	ComputeMarkers('/');
}

///////////////////////////////////////////////////////////////////////////////

void Path::AppendFolder( const char* folderappend )
{
    NameType Folder = GetFolder(EPATHTYPE_POSIX);
    Folder.append( folderappend, strlen(folderappend) );
    SetFolder( Folder.c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void Path::AppendFile( const char* fileappend )
{
    NameType File = GetName();
    File.append( fileappend, strlen(fileappend) );
    SetFile( File.c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void Path::SetFolder( const char* foldername )
{
    DecomposedPath decomposed;
	if( 0 == foldername )
	{   DeCompose( decomposed );
		decomposed.mFolder = "";
		Compose( decomposed );
	}
	else
	{   size_t newlen = strlen(foldername);
	    if( HasFolder() || (newlen>0) )
		{ DeCompose( decomposed );
			decomposed.mFolder.set( foldername );
			Compose( decomposed );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void Path::SetExtension( const char* newext )
{
    DecomposedPath decomposed;
	if( 0 == newext )
	{
        DeCompose( decomposed );
        decomposed.mExtension.set( "" );
        Compose( decomposed );
	}
	else
	{
		size_t newlen = strlen(newext);
		if( HasExtension() || (newlen>0) )
		{
            DeCompose( decomposed );
			decomposed.mExtension = (newext[0]=='.') ? (&newext[1]) : (newext);
            Compose( decomposed );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void Path::SetFile( const char* newfile )
{
    DecomposedPath decomposed;
	if( 0 == newfile )
	{   DeCompose( decomposed );
        decomposed.mFile.set( "" );
        Compose( decomposed );
	}
	else
	{   size_t newlen = strlen(newfile);
		if( HasFile() || (newlen>0) )
		{   DeCompose( decomposed );
            decomposed.mFile.set( newfile );
            Compose( decomposed );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

Path::EPathType Path::GetNative()
{
	return EPATHTYPE_POSIX;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasDrive() const
{
	return mMarkers.mDriveLen>0;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasUrlBase() const
{
    return mMarkers.mProtocolBaseLen>0;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasFolder() const
{
    return mMarkers.mFolderLen>0;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasQueryString() const
{
    return mMarkers.mQueryStringLen>0;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasExtension() const
{
    return mMarkers.mExtensionLen>0;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::HasFile() const
{
    return mMarkers.mFileNameLen>0;
}

///////////////////////////////////////////////////////////////////////////////

/*bool Path::IsAbsolute() const
{
	////////////////
	const char* instr = c_str();
	int ilen = int(strlen(instr));
	bool bleadingslash = (ilen>0) ? instr[0]=='/' : false;
	////////////////

	return HasUrlBase()||HasDrive()||bleadingslash;
}

///////////////////////////////////////////////////////////////////////////////

bool Path::IsRelative() const
{
	return (IsAbsolute()==false);
}*/

///////////////////////////////////////////////////////////////////////////////
// relative means relative to the working folder
/*
Path Path::ToRelative( EPathType etype ) const
{
	Path rval = ToAbsoluteFolder(etype);
	rval += GetName();
	rval += GetExtension().c_str();
	return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path Path::ToAbsolute( EPathType etype ) const
{
        Path tmp = ToAbsoluteFolder(etype);
        Path rval;
        if( HasExtension() )
        {
                //OrkHeapCheck();
                char buffer[1024];

                Path::NameType fname = GetName();
                Path::SmallNameType fext = GetExtension();
                const char* ptmp = tmp.c_str();
                const char* pname = fname.c_str();
                const char* pext = fext.c_str();

                snprintf( buffer, sizeof(buffer), "%s%s.%s", ptmp,pname,pext );
                rval.mPathString = Path::NameType(&buffer[0]);//.format( "%s%s.%s",  );
                //OrkHeapCheck();
        }
        else
        {
                rval.mPathString.format( "%s%s", tmp.c_str(), GetName().c_str() );
        }

        switch( etype )
        {
            case EPATHTYPE_POSIX:
            {   Path::NameType nt = rval.mPathString;
                rval.mPathString.replace( nt.c_str(), '\\', '/' );
                rval.ComputeMarkers('/');
                break;
            }
            default:
                OrkAssert(false);
                break;
        }
        return rval;
}

Path Path::ToAbsoluteFolder( EPathType etype ) const
{
    OrkAssert( etype == EPATHTYPE_POSIX );

    Path rval;

    if( HasUrlBase() )
    {
        const char* pstr = CFileEnv::GetPathFromUrlExt( GetUrlBase().c_str() ).c_str();
        size_t ilen = strlen(pstr);

        bool b_ends_with_slash = pstr[ilen-1] == '/';

        rval.mPathString.format( b_ends_with_slash ? "%s" : "%s/", pstr );
    }
    else if( HasDrive() )
    {
        rval.mPathString.format( "%s", CFileEnv::GetPathFromUrlExt( GetDrive().c_str() ).c_str() );
    }
    else if( IsAbsolute() )
    {
        switch( etype )
        {
            case EPATHTYPE_DOS:
            {
                if( mMarkers.mDriveLen == 3 )
                    rval.mPathString.format( "%.3s", rval.mPathString.c_str() );
                break;
            }
            case EPATHTYPE_POSIX:
            {
                rval.mPathString.format( "/" );
                break;
            }
        }
    }
    else
    {
    }
    rval.mPathString += GetFolder(etype); // GetFolder already does tonative pathsep
    switch( etype )
    {
        case EPATHTYPE_DOS:
        {
            size_t irl = rval.mPathString.length();
            if( 0==irl )
            {
                Path tmp;
                tmp.mPathString.format( "%s%s", GetStartupDirectory().c_str(), rval.c_str() );
                rval = tmp;
            }
            size_t ilen = rval.mPathString.length();
            const char* begin = rval.mPathString.c_str();
            unix2dospathsep xform;
            for( size_t i=0; i<ilen; i++ )
            {
                rval.mPathString.SetChar( i, xform( begin[i] ) );
            }
            rval.ComputeMarkers('\\');
            break;
        }
        default:
        {
            rval.ComputeMarkers('/');
            break;
        }
    }
    //orkprintf( "Path<%s> AbsoluteFolder<%s>\n", c_str(), rval.c_str() );
    return rval;
}
*/

 ///////////////////////////////////////////////////////////////////////////////

Path::SmallNameType Path::GetUrlBase() const
{
    Path::SmallNameType rval;
    int ilen = int(mMarkers.mProtocolBaseLen);
    int ibas = int(mMarkers.GetProtocolBase());
    for( int i=0; i<ilen; i++ )
    {
        rval.SetChar( int(i),  c_str()[ibas+i] );
    }
    rval.SetChar(ilen,0);
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path::SmallNameType Path::GetDrive() const
{
    Path::SmallNameType rval;
    int ilen = mMarkers.mDriveLen;
    int ibas = mMarkers.GetDriveBase();
    for( int i=0; i<ilen; i++ )
    {
        rval.SetChar( i, c_str()[ibas+i] );
        //strncpy( rval.c_str(), c_str()+ibas, ilen );
    }
    rval.SetChar(ilen,0);
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path::NameType Path::GetName() const
{
    Path::NameType rval;
    int ilen = int(mMarkers.mFileNameLen);
    int ibas = int(mMarkers.GetFileNameBase());
    for( int i=0; i<ilen; i++ )
    {
        rval.SetChar( i, c_str()[ibas+i] );
        //strncpy( rval.c_str(), c_str()+ibas, ilen );
    }
    rval.SetChar(ilen,0);
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path::SmallNameType Path::GetExtension() const
{
    Path::SmallNameType rval;
    int ilen = int(mMarkers.mExtensionLen);
    int ibas = int(mMarkers.GetExtensionBase());
    for( int i=0; i<ilen; i++ )
    {
        rval.SetChar( i, c_str()[ibas+i] );
        //strncpy( rval.c_str(), c_str()+ibas, ilen );
    }
    rval.SetChar(ilen,0);
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path::NameType Path::GetQueryString() const
{
    if( mMarkers.mQueryStringLen )
    {
        //orkprintf( "yo\n" );
    }
    Path::NameType rval;
    int ilen = mMarkers.mQueryStringLen;
    int ibas = mMarkers.GetQueryStringBase();
    if( ilen )
    {
        for( int i=0; i<ilen; i++ )
        {
            rval.SetChar( i, c_str()[ibas+i] );
            //strncpy( rval.c_str(), c_str()+ibas, ilen );
        }
    }
    rval.SetChar( ilen, 0 );
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path::NameType Path::GetFolder(EPathType etype) const
{
    Path::NameType rval;
    int ilen = mMarkers.mFolderLen;
    int ibas = mMarkers.GetFolderBase();
    for( int i=0; i<ilen; i++ )
    {
        rval.SetChar( i, c_str()[ibas+i] );
        //strncpy( rval.c_str(), c_str()+ibas, ilen );
    }
    rval.SetChar( ilen, 0 );
    return rval;
}

///////////////////////////////////////////////////////////////////////////////

Path Path::StripBasePath(const NameType& base) const
{
    Path basePath(base);
    ork::Path::NameType thisString = this->c_str(); //ToAbsolute(EPATHTYPE_POSIX).c_str();
    ork::Path::NameType baseString = basePath.c_str(); //ToAbsolute(EPATHTYPE_POSIX).c_str();

    if(thisString.find(baseString) == 0)
        return Path(thisString.substr(baseString.length()).c_str());
    else
        return *this;
}

///////////////////////////////////////////////////////////////////////////////

static const char* FindLastCharBefore( const char* src, int isrclen, const char search, const char b4 )
{   const char* rval = 0;
    for( int i=0; i<isrclen; i++ )
    {   if( src[i] == search )
        {   rval = src+i;
        }
        else if( src[i] == b4 )
        {   return rval;
        }
    }
    return rval;
}

//////////////////////////////////////////////////////////////////////////////

void Path::ComputeMarkers(char pathsep)
{
	const char* instr = c_str();

	int ilen = int(strlen(instr));

	//////////////////////////////////////////////
	// find feature markers

	const char* qmark  = strstr( instr, "?" );
	const char* umark  = strstr( instr, "://" );
	const char* dmark  = strstr( instr, ":" );
	const char* pmark  = FindLastCharBefore( instr, ilen, '.', '?' );
	const char* lsmark = FindLastCharBefore( instr, ilen, pathsep, '?' );

	////////////////////////////////////////////
	// if . before last slash, then it is a folder . and not an ext .
	////////////////////////////////////////////
	if( pmark<lsmark )
	{
		pmark=0;
	}
	////////////////////////////////////////////

	const char* folder_end_slash = qmark ? lsmark
                                         : strrchr( instr, pathsep );
	//const char* folder_beg_slash = strchr( instr, '/' );

	mMarkers.mDriveLen = 0;
	mMarkers.mExtensionLen = 0;
	mMarkers.mFileNameLen = 0;
	mMarkers.mFolderLen = 0;
	mMarkers.mQueryStringLen = 0;
	mMarkers.mProtocolBaseLen = 0;

	int istate = 0;

	/////////////////////////////////////////////
	// compute initial state based on presences of certain characters
	/////////////////////////////////////////////

	if( umark ) istate=0;
	else if( dmark )
	{
		if( qmark )
		{
			if( dmark<qmark ) // colon before query sep ?
			{
				istate=1;
			}
		}
		else
		{
			istate=1;
		}
	}

	if( (0==qmark) && (0==umark) && (0==dmark) && (0==lsmark) )
	{
		istate = 3;
	}
	else if( (0!=lsmark) && (0==dmark) && (0==umark) )
	{
		istate = 2;
	}
	/////////////////////////////////////////////
	// simple length parsing here
	/////////////////////////////////////////////
	if( qmark )
	{
		mMarkers.mQueryStringLen = ilen - (qmark-instr) + 1;
	}
	/////////////////////////////////////////////
	// update marker loop
	/////////////////////////////////////////////
	int imarkerstart = 0;

	for( int ic=0; ic<ilen; ic++ )
	{
		const char* ch = instr+ic;

		switch( istate )
		{
			case 0: // url
				if( *ch == ':' )
				{
        			mMarkers.mProtocolBaseLen = ic+3;
        			imarkerstart = mMarkers.mProtocolBaseLen;
        			istate = 2;
        			ic = imarkerstart;
        			//folder_beg_slash = strchr( instr+imarkerstart, '/' );
				}
				break;
			case 1: // drive
				if( *ch == ':' )
				{
				    mMarkers.mDriveLen = ic+2;
				    imarkerstart = mMarkers.mDriveLen;
				    istate = 2;
				    ic = imarkerstart;
				    //folder_beg_slash = strchr( instr+imarkerstart, '/' );
				}
				break;
			case 2: // folder
			{
				intptr_t ilastslashp = (folder_end_slash-instr);

				if( imarkerstart )
				{
				    if( strchr( instr+imarkerstart, pathsep ) == 0 )
				    {
				        istate++;
				        ic-=2;
				        continue;
				    }
				}
				OrkAssert( folder_end_slash!=0 );
				if( *ch == pathsep && (ch==folder_end_slash) )
				{
				    mMarkers.mFolderLen = (ic+1-imarkerstart);
				    imarkerstart += mMarkers.mFolderLen;
				    istate = 3;
				}
				break;
			}
			case 3: // file
				if( pmark )
				{
				    mMarkers.mFileNameLen = (pmark-ch);
				    imarkerstart += mMarkers.mFileNameLen;
				    istate = 4;
				}
				else if( qmark )
				{
				}
				else
				{
				    mMarkers.mFileNameLen++;
				    imarkerstart++;
				}
				break;
			case 4: // ext
				if( qmark )
				{
				    mMarkers.mExtensionLen = (ilen-imarkerstart)-mMarkers.mQueryStringLen;
				    imarkerstart += mMarkers.mExtensionLen;
				    istate++;
				}
				else
				{
				    mMarkers.mExtensionLen = ilen-imarkerstart;
				    imarkerstart += mMarkers.mExtensionLen;
				    istate++;
				}
				break;
			case 5: // query
			{
				//if( qmark )
				//{
			        //mMarkers.mQueryStringLen = ilen - (qmark-instr);
				//}
				istate++;
				break;
			}
			case 6: // end
				break;
		}
	}

	int itot = mMarkers.mDriveLen + mMarkers.mProtocolBaseLen + mMarkers.mFolderLen + mMarkers.mFileNameLen + mMarkers.mExtensionLen + mMarkers.mQueryStringLen;

	OrkAssert( itot == ilen );

}

///////////////////////////////////////////////////////////////////////////////

void Path::Compose( const DecomposedPath& decomposed )
{
    size_t iul = decomposed.mProtocol.length();
    size_t idl = decomposed.mDrive.length();
    size_t ifl = decomposed.mFolder.length();
    size_t igl = decomposed.mFile.length();
    size_t iel = decomposed.mExtension.length();
    size_t iql = decomposed.mQuery.length();

    OrkAssert( false == ((iul>0)&&(idl>0)) );
    OrkAssert( (idl==0)||(idl==3) );

    NameType str;

    if( iul ) str.append( decomposed.mProtocol.c_str(), iul );
    if( idl ) str.append( decomposed.mDrive.c_str(), idl );
    if( ifl ) str.append( decomposed.mFolder.c_str(), ifl );
    if( igl ) str.append( decomposed.mFile.c_str(), igl );
    if( iel )
    {   str.append( ".", 1 );
        str.append( decomposed.mExtension.c_str(), iel );
    }
    if( iql )
    {   str.append( "?", 1 );
        str.append( decomposed.mQuery.c_str(), iql );
    }
    Set( str.c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void Path::DeCompose( DecomposedPath& decomposed )
{
	OrkAssert( false == (HasUrlBase()&&HasDrive()) );
	if( HasUrlBase() )
	{       //strncpy( url.c_str(), c_str()+mMarkers.GetUrlBase(), mMarkers.GetUrlLength() );
		//url.c_str()[ mMarkers.GetUrlLength() ] = 0;
		decomposed.mProtocol.set( c_str()+mMarkers.GetProtocolBase(), mMarkers.GetProtocolLength() );
		decomposed.mProtocol.SetChar( mMarkers.GetProtocolLength(), 0 );
	}
	else
	{
		//url.c_str()[0] = 0;
		decomposed.mProtocol.SetChar( 0, 0 );
	}
	if( HasDrive() )
	{
		//strncpy( drive.c_str(), c_str()+mMarkers.GetDriveBase(), mMarkers.GetDriveLength() );
		//drive.c_str()[ mMarkers.GetDriveLength() ] = 0;
		decomposed.mDrive.set( c_str()+mMarkers.GetDriveBase(), mMarkers.GetDriveLength() );
		decomposed.mDrive.SetChar( mMarkers.GetDriveLength(), 0 );
	}
	else
	{
		//drive.c_str()[0] = 0;
		decomposed.mDrive.SetChar( 0, 0 );
	}
	if( HasFolder() )
	{
		//strncpy( folder.c_str(), c_str()+mMarkers.GetFolderBase(), mMarkers.GetFolderLength() );
		//folder.c_str()[ mMarkers.GetFolderLength() ] = 0;
		decomposed.mFolder.set( c_str()+mMarkers.GetFolderBase(), mMarkers.GetFolderLength() );
		decomposed.mFolder.SetChar( mMarkers.GetFolderLength(), 0 );
	}
	else
	{
		//folder.c_str()[0] = 0;
		decomposed.mFolder.SetChar( 0, 0 );
	}
	if( HasFile() )
	{
		//strncpy( file.c_str(), c_str()+mMarkers.GetFileNameBase(), mMarkers.GetFileNameLength() );
		//file.c_str()[ mMarkers.GetFileNameLength() ] = 0;
		decomposed.mFile.set( c_str()+mMarkers.GetFileNameBase(), mMarkers.GetFileNameLength() );
		decomposed.mFile.SetChar( mMarkers.GetFileNameLength(), 0 );
	}
	else
	{
		//file.c_str()[0] = 0;
		decomposed.mFile.SetChar( 0, 0 );
	}
	if( HasExtension() )
	{
		//strncpy( ext.c_str(), c_str()+mMarkers.GetExtensionBase(), mMarkers.GetExtensionLength() );
		//ext.c_str()[ mMarkers.GetExtensionBase() ] = 0;
		int ibase = mMarkers.GetExtensionBase();
		decomposed.mExtension.set( c_str()+ibase, mMarkers.GetExtensionLength() );
		decomposed.mExtension.SetChar( mMarkers.GetExtensionLength(), 0 );
	}
	else
	{
		decomposed.mExtension.SetChar( 0, 0 );
		//ext.c_str()[0] = 0;
	}
	if( HasQueryString() )
	{
		//strncpy( query.c_str(), c_str()+mMarkers.GetQueryStringBase(), mMarkers.GetQueryStringLength() );
		//query.c_str()[ mMarkers.GetQueryStringLength() ] = 0;
		decomposed.mQuery.set( c_str()+mMarkers.GetQueryStringBase(), mMarkers.GetQueryStringLength() );
		decomposed.mQuery.SetChar( mMarkers.GetQueryStringLength(), 0 );
	}
	else
	{
		decomposed.mQuery.SetChar( 0, 0 );
		//query.c_str()[0] = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////

void Path::SplitQuery( NameType& preq, NameType& postq ) const
{
	if( HasQueryString() )
	{
		unsigned qpos = mMarkers.GetQueryStringBase();
		preq.SetChar( 0,0 );
		preq.append( c_str(), int(qpos-1));
		postq.SetChar(0,0);
		postq.append( c_str()+qpos, int(mMarkers.GetQueryStringLength()));
	}
	else
	{
		preq = mPathString;
		postq.SetChar(0,0);
	}
}

///////////////////////////////////////////////////////////////////////////////

bool Path::DoesPathExist() const
{
    struct stat file_stat;
    int ist = stat( c_str(), & file_stat );
    printf( "stat<%s> : %d\n", c_str(), ist );
    return (ist==0);
}

bool Path::IsFile() const
{
    struct stat file_stat;
    int ist = stat( c_str(), & file_stat );
    printf( "stat<%s> : %d\n", c_str(), ist );
    return (ist==0) ? bool(S_ISREG(file_stat.st_mode)) : false;

}
bool Path::IsFolder() const
{
    struct stat file_stat;
    int ist = stat( c_str(), & file_stat );
    printf( "stat<%s> : %d\n", c_str(), ist );
    return (ist==0) ? bool(S_ISDIR(file_stat.st_mode)) : false;
}
bool Path::IsSymLink() const
{
    struct stat file_stat;
    int ist = stat( c_str(), & file_stat );
    printf( "stat<%s> : %d\n", c_str(), ist );
    return (ist==0) ? bool(S_ISLNK(file_stat.st_mode)) : false;
}

///////////////////////////////////////////////////////////////////////////////
size_t Path::SizeOfFile() const
{
    struct stat file_stat;
    int ist = stat( c_str(), & file_stat );
    size_t rval = (ist==0)
        ? size_t(file_stat.st_size)
        : false;
    //printf( "sizeof<%s> : %d\n", c_str(), int(rval) );
    return rval;
}

} // namespace