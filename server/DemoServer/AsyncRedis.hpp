#ifndef MYASYNCREDIS_HPP
#define MYASYNCREDIS_HPP
//#define WIN32_LEAN_AND_MEAN// make sure all macros are included 
#include <windows.h>
#include <winsock2.h>
#include <event.h>
#include <evhttp.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")
#pragma comment(lib, "hiredis.lib")

std::mutex g_SubscribeMutex;

void RedisCommandAdapter(char* value)
{
	printf("Replay %s\n", value);
}

void subscribeCallback(redisAsyncContext *context, void *replay, void *priv) {
	g_SubscribeMutex.lock();
	redisReply *reply = static_cast<redisReply*>(replay);
	if (reply == NULL) return;
	if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
		if (strcmp(reply->element[0]->str, "subscribe") != 0) {
			if (NULL != g_pServer)
			{
				g_pServer->RedisSubscribeAsync((char*)priv, reply->element[1]->str, reply->element[2]->str);
			}
		
		}
	}
	g_SubscribeMutex.unlock();
}


void connectCallback(const redisAsyncContext *context, int status) {
	g_SubscribeMutex.lock();
	if (status != REDIS_OK) {
		printf("Error: %s\n", context->errstr);
		return;
	}
	printf("Redis connected...\n");
	g_SubscribeMutex.unlock();
}

void disconnectCallback(const redisAsyncContext *context, int status) {
	g_SubscribeMutex.lock();
	if (status != REDIS_OK) {
		printf("Error: %s\n", context->errstr);
		return;
	}
	printf("Disconnected...\n");
	g_SubscribeMutex.unlock();
}

void authCallback(redisAsyncContext *context, void *replay, void *priv)
{
	g_SubscribeMutex.lock();
	redisReply *reply = static_cast<redisReply*>(replay);
	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		RedisCommandAdapter("auth OK");
	}
	g_SubscribeMutex.unlock();
}

void func(const std::string& commandStr)
{
	g_SubscribeMutex.lock();
	struct event_base *m_base = event_base_new();

	redisAsyncContext *m_redis = redisAsyncConnect("127.0.0.1", 6379);
	if (m_redis->err) {
		/* Let *c leak for now... */
		printf("Error: %s\n", m_redis->errstr);
	}
	redisAsyncCommand(m_redis,authCallback, (char*) "auth", "auth %s", "014006");

	redisLibeventAttach(m_redis, m_base);
	redisAsyncSetConnectCallback(m_redis, connectCallback);
	redisAsyncSetDisconnectCallback(m_redis, disconnectCallback);

	redisAsyncCommand(m_redis, subscribeCallback, (char*) "sub", commandStr.c_str());
	g_SubscribeMutex.unlock();
	event_base_dispatch(m_base);
}
#endif //MYASYNCREDIS_HPP