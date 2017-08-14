//
#include <unittest++/UnitTest++.h>
#include <ork/path.h>

///////////////////////////////////////////////////////////////////////////////

TEST(PathCanComposeAndDecomposeUrlPaths)
{
	ork::DecomposedPath decomp;
    ork::Path p1("testaa://archetypes/yo.txt");
    p1.DeCompose(decomp);
    ork::Path p2;
    p2.Compose(decomp);
    CHECK(p1==p2);
}

///////////////////////////////////////////////////////////////////////////////

TEST(PathCorrectlyReturnsTheNamePartOfAPath)
{
    ork::Path testPath("/hello/world/test.txt");
    ork::Path testPath2("/hello/world/");
    CHECK_EQUAL("test", testPath.GetName().c_str());
    CHECK_EQUAL("", testPath2.GetName().c_str());
}

///////////////////////////////////////////////////////////////////////////////

TEST(PathCorrectlyReturnsTheExtensionPartOfAPath)
{
	ork::Path testPath("/hello/world/test.txt");
	ork::Path testPath2("/hello/world/");
	ork::Path testPath3("/hello/world/test");

	CHECK_EQUAL("txt", testPath.GetExtension().c_str());
	CHECK_EQUAL("", testPath2.GetExtension().c_str());
	CHECK_EQUAL("", testPath3.GetExtension().c_str());
}

///////////////////////////////////////////////////////////////////////////////

TEST(PathCanStoreQueryStrings)
{
    ork::Path testPath("testaa://hello/world/test.txt?yo=dude");
    CHECK_EQUAL( true, testPath.HasQueryString() );
    CHECK_EQUAL( "yo=dude", testPath.GetQueryString().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

TEST(PathCanNotStoreQueryStrings)
{
    ork::Path testPath("testaa://hello/world/test.txt");
    CHECK_EQUAL( false, testPath.HasQueryString() );
    CHECK_EQUAL( "", testPath.GetQueryString().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

TEST(PathHostNameTest)
{
    ork::Path p1("http://localhost:5901/yo.txt");
    ork::DecomposedPath decomp;
    p1.DeCompose(decomp);
    CHECK_EQUAL( "localhost", decomp.mHostname.c_str() );
    // we know this fails now, I am working on it!
    //printf( "hname<%s>\n", decomp.mHostname.c_str() );
    
}

///////////////////////////////////////////////////////////////////////////////
