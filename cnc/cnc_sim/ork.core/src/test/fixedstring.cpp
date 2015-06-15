#include <unittest++/UnitTest++.h>
#include <ork/fixedstring.h>
#include <string.h>
#include <unordered_set>
#include <set>

using namespace ork;

TEST(fixedstringassignentfromcharstar)
{
	fixedstring<4096> the_string;
	the_string = "what<0>up<1>yo";
	CHECK(0==strcmp(the_string.c_str(),"what<0>up<1>yo"));
}

TEST(fixedstringTestFormat)
{
	fixedstring<4096> the_string;
	the_string.format( "what<%d>up<%d>%s", 0, 1, "yo" );
	CHECK(0==strcmp(the_string.c_str(),"what<0>up<1>yo"));
}

TEST(fixedstringTestReplace1)
{
	fixedstring<4096> src_string("whatupyodiggittyyoyo");
	fixedstring<4096> dst_string;

	dst_string.replace( src_string.c_str(), "yo", "damn" );
	//printf( "dst_string<%s>\n", dst_string.c_str() );
	CHECK(0==strcmp(dst_string.c_str(),"whatupdamndiggittydamndamn"));
}

TEST(fixedstringTestFind1)
{
	fixedstring<4096> src_string("whatupyodiggittyyoyo");
	size_t f = src_string.find_first_of( "diggit" );
	CHECK(f==size_t(8));
}

TEST(fixedstringTestFind2)
{
	fixedstring<4096> src_string("whatupyodiggittyyoyo");
	size_t f = src_string.find_first_of( "yo" );
	CHECK(f==size_t(6));
}

TEST(fixedstringTestFind3)
{
	fixedstring<4096> src_string("whatupyodiggittyyoyo");
	size_t f = src_string.find_last_of( "yo" );
	CHECK(f==size_t(18));
}

TEST(fixedstringTestSize1)
{
	fixedstring<4> src_string("wha");
	CHECK(src_string.size()==3);
}

TEST(fixedstringTestSize2)
{
	fixedstring<4> src_string;
	src_string.format("what");
	CHECK(src_string.size()==3); // 3 because it will get truncated due to length constraints
}

TEST(fixedstringHashSetCompare1)
{
	std::unordered_set<fixedstring<256>> the_set;
	the_set.insert("yo");
	the_set.insert("what");
	the_set.insert("up");
	the_set.insert("yo");
	the_set.insert("what");
	the_set.insert("up");
	CHECK(the_set.size()==3); 
}

TEST(fixedstringOrderedSetCompare1)
{
	std::set<fixedstring<256>> the_set;
	the_set.insert("yo");
	the_set.insert("what");
	the_set.insert("up");
	the_set.insert("yo");
	the_set.insert("what");
	the_set.insert("up");
	CHECK(the_set.size()==3); 
}

TEST(fixedstringMixedConcat)
{
	fixedstring<32> a("whatup");
	fixedstring<64> b("yo");

	a += b;

	CHECK(0==strcmp(a.c_str(),"whatupyo")); 
}
