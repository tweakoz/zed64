///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <ork/fixedstring.h>

namespace ork {

//////////////////////////////////////////////////////////

//! convert DOS filesystem path node delimiters to UNIX
struct dos2unixpathsep
{ inline char operator() (char c) { return (c=='\\') ? '/' : c; }
};
//! convert UNIX filesystem path node delimiters to DOS
struct unix2dospathsep
{ inline char operator() (char c) { return (c=='/') ? '\\' : c; }
};

//////////////////////////////////////////////////////////

class Path;

//! set of markers on a filesystem path delimiting various sections of that path
class PathMarkers
{
    friend class Path;

    unsigned int mProtocolBaseLen           : 5; // 5
    unsigned int mDriveLen                  : 2; // 7
    unsigned int mHostNameLen               : 8; // 15
    unsigned int mPortLen                   : 3; // 23
    unsigned int mFolderLen                 : 8; // 26
    unsigned int mFileNameLen               : 8; // 34
    unsigned int mExtensionLen              : 4; // 38
    unsigned int mQueryStringLen            : 5; // 43

public:

    unsigned int GetProtocolBase() const;
    unsigned int GetDriveBase() const;
    unsigned int GetHostNameBase() const;
    unsigned int GetPortBase() const;
    unsigned int GetFolderBase() const;
    unsigned int GetFileNameBase() const;
    unsigned int GetExtensionBase() const;
    unsigned int GetQueryStringBase() const;

    unsigned int GetProtocolLength() const { return mProtocolBaseLen; }
    unsigned int GetDriveLength() const { return mDriveLen; }
    unsigned int GetHostNameLength() const { return mHostNameLen; }
    unsigned int GetPortLength() const { return mPortLen; }
    unsigned int GetFolderLength() const { return mFolderLen; }
    unsigned int GetFileNameLength() const { return mFileNameLen; }
    unsigned int GetExtensionLength() const { return mExtensionLen; }
    unsigned int GetQueryStringLength() const { return mQueryStringLen; }

    PathMarkers();
};

//////////////////////////////////////////////////////////

//! filesystem path decomposed into its various comoponents
struct DecomposedPath
{
    typedef fixedstring<256> string_t;

    string_t mProtocol;
    string_t mHostname;
    string_t mPort;

    string_t mDrive;
    string_t mFolder;
    string_t mFile;
    string_t mExtension;
    string_t mQuery;
};

//////////////////////////////////////////////////////////

//! filesystem path manipulation class
class Path
{
        public:

        typedef uint32_t HashType;

        typedef fixedstring<32> SmallNameType;
        typedef fixedstring<256> NameType;

        enum EPathType
        {
            EPATHTYPE_POSIX,
            EPATHTYPE_URL,
            EPATHTYPE_ASSET=EPATHTYPE_URL,
        };

        Path();
        Path(const std::string& pathName);
        Path(const char* pathName);
        Path(const NameType& pathName);

        ~Path();

        //////////////////////////////////////////////

        void operator = ( const Path& oth );
        bool operator == ( const Path& oth ) const;
        bool operator != ( const Path& oth ) const;
        void operator += ( const Path& oth );
        bool operator < ( const Path& oth ) const;
        Path operator + ( const Path& oth ) const;
        size_t length() const;
        bool empty() const;

        //////////////////////////////////////////////

        static EPathType GetNative();

        void SetFile(const char* filename);
        void AppendFile(const char* filename);
        void SetFolder(const char* pathName);
        void AppendFolder(const char* filename);
        void SetExtension(const char* ext);
        void SetUrlBase(const char* UrlBase);
        void SetDrive(const char* UrlBase);

        void Set(const char* pathName);

        bool IsAbsolute() const;
        bool IsRelative() const;
        bool HasUrlBase() const;
        bool HasFolder() const;
        bool HasDrive() const;
        bool HasQueryString() const;
        bool HasExtension() const;
        bool HasFile() const;

        bool DoesPathExist() const;
        bool IsFile() const;
        bool IsFolder() const;
        bool IsSymLink() const;

        //////////////////////////////////////////////

        void DeCompose(	DecomposedPath& decomposed );
        void Compose( const DecomposedPath& decomposed );

        void ComputeMarkers( char pathsep );

        //////////////////////////////////////////////

        void SplitQuery( NameType& BeforeQuerySep, NameType& AfterQuerySep ) const;

        //////////////////////////////////////////////
        size_t SizeOfFile() const;
        //////////////////////////////////////////////

        SmallNameType GetDrive() const;
        SmallNameType GetExtension() const;
        SmallNameType GetUrlBase() const;

        NameType GetName() const;
        NameType GetQueryString() const;
        NameType GetFolder(EPathType etype) const;

        //////////////////////////////////////////////

        Path StripBasePath(const NameType& base) const;

        const char* c_str() const { return mPathString.c_str(); }

        //HashType Hash() const;

        private:

        //////////////////////////////////////

        NameType        mPathString;
        PathMarkers     mMarkers;

        //////////////////////////////////////

};

//////////////////////////////////////////////////////////
};