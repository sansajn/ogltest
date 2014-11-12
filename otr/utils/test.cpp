#include <string>

#define BOOST_TEST_MODULE UtilittiesTest
#include <boost/test/included/unit_test.hpp>

#include "core/utils.hpp"

using namespace std;

BOOST_AUTO_TEST_CASE( tokenizer_test )
{
	char const * expected[] = {"hello", "jane"};

	std::string text = "hello,,jane,,,";
	int count = 0;
	for (auto t : tokenizer(text, ","))
		BOOST_CHECK_EQUAL(t, expected[count++]);
}

BOOST_AUTO_TEST_CASE( memory_stream_test )
{
	memory_stream mout(10*sizeof(int));
	for (int i = 0; i < 10; ++i)
		mout << i*i;

	int * ibuf = mout.buf<int>();;
	for (int i = 0; i < 10; ++i)
		BOOST_CHECK_EQUAL(ibuf[i], i*i);
}

BOOST_AUTO_TEST_CASE( memory_steam_hetero_test )
{
	memory_stream mout(10*sizeof(int)+3*sizeof(double));
	for (int i = 0; i < 10; ++i)
		mout << i*i;

	for (int i = 0; i < 3; ++i)
		mout << (i+1)*1.11;

	int * ibuf = mout.buf<int>();
	for (int i = 0; i < 10; ++i)
		BOOST_CHECK_EQUAL(ibuf[i], i*i);

	double * dbuf = mout.buf<double>(10*4);
	for (int i = 0; i < 3; ++i)
		BOOST_CHECK_EQUAL(dbuf[i], (i+1)*1.11);
}

struct vec3
{
	float x, y, z;
	vec3 & operator=(vec3 const & rhs) = delete;
};

memory_stream & operator<<(memory_stream & out, vec3 const & v)
{
	out << v.x << v.y << v.z;
	return out;
}

BOOST_AUTO_TEST_CASE( memory_stream_custom_structure_test )
{
	memory_stream mout(3*sizeof(vec3));
	mout << vec3{1.11, 2.22, 3.33} << vec3{0.11, 0.22, 0.33} << vec3{11.1, 22.1, 33.3};

	float expected[9] = {1.11, 2.22, 3.33, 0.11, 0.22, 0.33, 11.1, 22.1, 33.3};

	float * fbuf = mout.buf<float>();
	for (int i = 0; i < 9; ++i)
		BOOST_CHECK_EQUAL(fbuf[i], expected[i]);
}

BOOST_AUTO_TEST_CASE( file_reading_test )
{
	string content(read_file("data/simple.txt"));
	BOOST_CHECK_EQUAL(content, string("hello lisbon!"));
}
