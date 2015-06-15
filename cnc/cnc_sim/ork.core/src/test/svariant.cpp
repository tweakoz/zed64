#include <unittest++/UnitTest++.h>
#include <ork/svariant.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

struct my_yo
{
	std::string mOne;
	std::string mTwo;
	std::string mThree;
	std::string mFour;
	std::string mFive;
	std::string mSix;
};

typedef ork::svar256_t svar_t;

TEST(svar_non_pod)
{
	my_yo the_yo;
	svar_t v(the_yo);
	svar_t vb;

	printf( "sizeof<my_yo:%d>\n", int(sizeof(my_yo)));

	vb = v;
}


