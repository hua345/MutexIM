#ifndef MYSESSIONID_HPP
#define MYSESSIONID_HPP

#include <random>
#include <time.h>
#include <openssl/md5.h>

#pragma comment(lib, "libeay32.lib")  
#pragma comment(lib, "ssleay32.lib") 

static  std::default_random_engine g_generator(314159);//使用给定的种子;
static  std::uniform_int_distribution<int> g_uniformDistribution(48, 122);//0(48),z(122)

static const std::string getSessionID()
{
	time_t nowtime;
	time(&nowtime);
	char buf1[15];
	_itoa((int32_t)nowtime, buf1, 10);
	//sprintf(randomArr, "%d", nowtime);
	for (int i = strlen(buf1); i < 15; ++i)
	{
		buf1[i] =  static_cast<char>(g_uniformDistribution(g_generator));
	}
	// generates number in the range 1..100000	
	return std::string((char*)buf1, 15);
};
#endif