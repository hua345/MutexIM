#include "stdafx.h"
#include "RedisSubscribe.h"

RedisSubscribe::RedisSubscribe(const std::string& subscribeName)
: m_base(NULL)
, m_redis(NULL)
{
	g_ptr = this;
	m_base = event_base_new();
	m_redis = redisAsyncConnect("127.0.0.1", 6379);
	if (m_redis->err) {
		/* Let *c leak for now... */
		printf("Error: %s\n", m_redis->errstr);
	}
	redisAsyncCommand(m_redis, &RedisSubscribe::authCallback, (char*) "sub", "auth %s", "014006");

	redisLibeventAttach(m_redis, m_base);
	redisAsyncSetConnectCallback(m_redis, &RedisSubscribe::connectCallback);
	redisAsyncSetDisconnectCallback(m_redis, &RedisSubscribe::disconnectCallback);
	redisAsyncCommand(m_redis, &RedisSubscribe::subscribeCallback, (char*) "sub", "SUBSCRIBE %s", subscribeName.c_str());
}


RedisSubscribe::~RedisSubscribe()
{
}

void RedisSubscribe::subscribeCallback(redisAsyncContext *context, void *replay, void *priv)
{
	redisReply *reply = static_cast<redisReply*>(replay);
	if (reply == NULL) return;
	if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
		if (strcmp(reply->element[0]->str, "subscribe") != 0) {
			printf("Received[%s] channel %s: %s\n",
				(char*)priv,
				reply->element[1]->str,
				reply->element[2]->str);
			test(g_ptr, std::string(reply->element[2]->str));
		}
	}
}

void RedisSubscribe::connectCallback(const redisAsyncContext *context, int status)
{
	if (status != REDIS_OK) {
		printf("Error: %s\n", context->errstr);
		return;
	}
	printf("Redis connected...\n");
}

void RedisSubscribe::disconnectCallback(const redisAsyncContext *context, int status)
{
	if (status != REDIS_OK) {
		printf("Error: %s\n", context->errstr);
		return;
	}
	printf("Disconnected...\n");
}

void RedisSubscribe::authCallback(redisAsyncContext *context, void *replay, void *priv)
{
	redisReply *reply = static_cast<redisReply*>(replay);
	if (reply == NULL) return;
	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		printf("auth OK\n");
	}
}

void RedisSubscribe::test( RedisSubscribe* redis, const std::string& str)
{
	redis->test2(str);
}

void RedisSubscribe::test2(const std::string& str)
{
	std::cout << str.c_str() << std::endl;
}

void RedisSubscribe::run()
{
	//event_base_dispatch(m_base);
	std::thread m_thread(std::bind(event_base_dispatch,m_base));
	m_thread.detach();
}
