#pragma once
#include <functional>
#include <thread>
#include <winsock2.h>
#include <event.h>
#include <evhttp.h>
#include <stdlib.h>
#include <signal.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <iostream>

#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")
#pragma comment(lib, "hiredis.lib")

class RedisSubscribe;

static RedisSubscribe* g_ptr = NULL;
class RedisSubscribe
{
public:
	RedisSubscribe(const std::string& subscribeName);
	~RedisSubscribe();
	static void subscribeCallback(redisAsyncContext *context, void *replay, void *priv);
	static void connectCallback(const redisAsyncContext *context, int status);
	static void disconnectCallback(const redisAsyncContext *context, int status);
	static void authCallback(redisAsyncContext *context, void *replay, void *priv);
	static void test(RedisSubscribe* redis, const std::string& str);
	void test2(const std::string& str);
	void run();
private:
	struct event_base *m_base;
	redisAsyncContext *m_redis;
};

