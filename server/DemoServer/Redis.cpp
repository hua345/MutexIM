#include "stdafx.h"
#include "Redis.h"

CRedis::CRedis()
:m_ptrContext(NULL)
, m_ptrReply(NULL)
{

}

CRedis::~CRedis()
{
	//释放redisConnect()所产生的连接
	redisFree(m_ptrContext);
	this->m_ptrContext = NULL;
	this->m_ptrReply = NULL;
}
void CRedis::InitRedis()
{
	if (!Connect(RedisIP, RedisPort))
	{
		printf("connect error!\n");
	}
	if (Auth(RedisPWD).compare("OK") != 0) // 0 They compare equal
	{
		printf("auth error!\n");
	}
	if (SelectDB(1).compare("OK") != 0)// 0 They compare equal
	{
		printf("select db error!\n");
	}
}

vector<string> CRedis::SortMultiSocket(const string& groupName)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "SORT %s by no_exist_key get # get online:*", groupName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements/2; ++i)
		{
			if (0 != m_ptrReply->element[2 * i + 1]->len)
			{
				vecRes.push_back(m_ptrReply->element[2 * i]->str);
				vecRes.push_back(m_ptrReply->element[2 * i + 1]->str);
			}
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

bool CRedis::Connect(string host, int port)
{
	this->m_ptrContext = redisConnect(host.c_str(), port);
	if (this->m_ptrContext != NULL && this->m_ptrContext->err)
	{
		printf("connect error: %s\n", this->m_ptrContext->errstr);
		return 0;
	}
	return 1;
}

std::string CRedis::Get(const string& key)
{
	std::string str;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "GET %s", key.c_str()));
	//对于不存在key的返回值，其类型为REDIS_REPLY_NIL。
	if (NULL != m_ptrReply && REDIS_REPLY_STRING == m_ptrReply->type && NULL != m_ptrReply->str)
	{
		str += string(m_ptrReply->str, m_ptrReply->len);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return str;
}

int CRedis::DEL(const string& key)
{
	int nRes = 0;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "DEL %s", key.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

vector<string> CRedis::MGet(const vector<string>& vecKey)
{
	vector<string> vecRes;
	if (vecKey.size() == 0) return vecRes;
	//拼接字符串;
	string commandStr("MGET ");
	auto itBegin = vecKey.begin();
	while (itBegin != vecKey.end())
	{
		commandStr += *itBegin;
		commandStr += " ";
		++itBegin;
	}
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, commandStr.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(string(m_ptrReply->element[i]->str, m_ptrReply->element[i]->len));
		}

	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

bool CRedis::Exists(const string& key)
{
	bool bRes = false;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "EXISTS  %s", key.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		bRes = (1 == m_ptrReply->integer);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return bRes;
}

int CRedis::HDel(const string& hashMapName, const vector<string>& vecStr)
{
	int nRes = 0;
	//拼接字符串;
	string commandStr("HDEL ");
	commandStr += hashMapName;
	commandStr += " ";
	auto itBegin = vecStr.begin();
	while (itBegin != vecStr.end())
	{
		commandStr += *itBegin;
		commandStr += " ";
		++itBegin;
	}
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, commandStr.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

int CRedis::HDel(const string& hashMapName, const string& key)
{
	int nRes = 0;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "HDEL %s %s", hashMapName.c_str(), key.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

std::string CRedis::HGet(const string& hashMapName, const string& key)
{
	std::string str;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "HGET %s %s", hashMapName.c_str(), key.c_str()));
	if (REDIS_REPLY_STRING == m_ptrReply->type && NULL != m_ptrReply->str)
	{
		str += string(m_ptrReply->str, m_ptrReply->len);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return str;
}

map<string, string> CRedis::HMGet(const string& hashMapName, const vector<string>& vecStr)
{
	map<string, string> map_;
	if (vecStr.size() == 0) return map_;
	//拼接字符串;
	string commandStr("HMGET ");
	commandStr += hashMapName;
	commandStr += " ";
	auto itBegin = vecStr.begin();
	while (itBegin != vecStr.end())
	{
		commandStr += *itBegin;
		commandStr += " ";
		++itBegin;
	}
	//执行命令;
	this->m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, commandStr.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type && 0 != m_ptrReply->elements)
	{
		for (size_t i = 0; i < m_ptrReply->elements; ++i) {
			map_[vecStr[i]] = string(m_ptrReply->element[i]->str, m_ptrReply->element[i]->len);
		}
	}
	else if (REDIS_REPLY_ERROR == m_ptrReply->type)
	{
		//error handle
	}

	return map_;
}

bool CRedis::HGetAll(const string& hashMapName, map<string, string>& dataMap)
{
	bool bRes = false;
	this->m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "HGETALL %s", hashMapName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type && 0 != m_ptrReply->elements)
	{
		bRes = true;
		for (size_t i = 0; i < m_ptrReply->elements / 2; ++i) {
			dataMap[m_ptrReply->element[2 * i]->str] = string(m_ptrReply->element[2 * i + 1]->str, m_ptrReply->element[2 * i + 1]->len);
		}
		
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(this->m_ptrReply);
	return bRes;
}

std::string CRedis::SelectDB(int databaseId)
{
	std::string str;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "select %d", databaseId));
	if (NULL != m_ptrReply && REDIS_REPLY_STATUS == m_ptrReply->type  && NULL != m_ptrReply->str)
	{
		str += string(m_ptrReply->str, m_ptrReply->len);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return str;
}

bool CRedis::HExists(const string& hashMapName, const string& key)
{
	bool bRes = false;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "HEXISTS %s %s", hashMapName.c_str(), key.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		bRes = (1 == m_ptrReply->integer);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return bRes;
}

std::string CRedis::Set(const string& key, const string& value)
{
	std::string str;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "SET %s %s", key.c_str(), value.c_str()));
	if (NULL != m_ptrReply && NULL != m_ptrReply->str)
	{
		str += string(m_ptrReply->str, m_ptrReply->len);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return str;
}


int CRedis::HSet(const string& hashMapName, const string& key, const string& value)
{
	int nResult = 0;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "HSET %s %s %s", hashMapName.c_str(), key.c_str(), value.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nResult = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nResult;
}


std::string CRedis::HMSet(const string& hashMapName, const map<string, string>& mapValue)
{
	string strResult;
	if (mapValue.size() == 0) return strResult;
	//拼接字符串;
	string commandStr("HMSET ");
	commandStr += hashMapName;
	commandStr += " ";
	auto itBegin = mapValue.begin();
	while (itBegin != mapValue.end())
	{
		commandStr += itBegin->first;
		commandStr += " ";
		commandStr += itBegin->second;
		commandStr += " ";
		++itBegin;
	}
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, commandStr.c_str()));
	//字符串类型的set命令的返回值的类型是REDIS_REPLY_STATUS，然后只有当返回信息是"OK"
	if (NULL != m_ptrReply && REDIS_REPLY_STATUS == m_ptrReply->type)
	{
		strResult = string(m_ptrReply->str, m_ptrReply->len);
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return strResult;
}

vector<string> CRedis::LRANGE_binary(const string& listName, int begin, int end)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "LRANGE %s %d %d", listName.c_str(), begin, end));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(string(m_ptrReply->element[i]->str,m_ptrReply->element[i]->len));
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

vector<string> CRedis::SMEMBERS(const string& SetsName)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "SMEMBERS %s", SetsName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(string(m_ptrReply->element[i]->str, m_ptrReply->element[i]->len));
		}

	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

int CRedis::SADD(const string& setName, const string& memberName)
{
	int nRes = 0;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "SADD %s  %s", setName.c_str(), memberName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

int CRedis::SREM(const string& setName, const string& memberName)
{
	int nRes = 0;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "SREM %s  %s", setName.c_str(), memberName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

int CRedis::ZADD(const string& zsetName, int score, const string& memberName)
{
	int nRes = 0;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZADD %s %d %s", zsetName.c_str(), score, memberName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

int CRedis::ZREM(const string& zsetName, const string& memberName)
{
	int nRes = 0;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZREM %s  %s", zsetName.c_str(), memberName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

std::string CRedis::ZINCRBY(const string& zsetName, int changeScore, const string& memberName)
{
	string strRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZINCRBY %s %d %s", zsetName.c_str(), changeScore, memberName.c_str()));
	if (NULL != m_ptrReply && REDIS_REPLY_STRING == m_ptrReply->type)
	{
		strRes = m_ptrReply->str;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return strRes;
}

vector<string> CRedis::ZRANGE(const string& zsetName, int begin, int end)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZRANGE %s %d %d", zsetName.c_str(), begin, end));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(m_ptrReply->element[i]->str);
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

vector<string> CRedis::ZREVRANGE(const string& zsetName, int begin, int end)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZREVRANGE %s %d %d", zsetName.c_str(), begin, end));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(m_ptrReply->element[i]->str);
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

vector<string> CRedis::ZRANGEBYSCORE(const string& zsetName, int beginScore, int endScore)
{
	vector<string> vecRes;
	//执行命令;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "ZRANGEBYSCORE %s %d %d", zsetName.c_str(), beginScore, beginScore));
	if (NULL != m_ptrReply && REDIS_REPLY_ARRAY == m_ptrReply->type)
	{
		for (int i = 0; i < m_ptrReply->elements; ++i)
		{
			vecRes.push_back(m_ptrReply->element[i]->str);
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return vecRes;
}

int CRedis::Publish(const string& channel, const string& content)
{
	//send binary data;
	int nRes = 0;
	m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "PUBLISH %s %b", channel.c_str(), content.c_str(), content.size()));
	if (NULL != m_ptrReply && REDIS_REPLY_INTEGER == m_ptrReply->type)
	{
		nRes = m_ptrReply->integer;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return nRes;
}

void  CRedis::Subscribe(const string& subCommand, TSubscribeCallback callback)
{
	redisContext* ptrContext = NULL;			//连接上下文指针;
	redisReply*	  ptrReply = NULL;			//操作结果指针;
	//连接数据库;
	ptrContext = redisConnect(RedisIP, RedisPort);
	if (ptrContext != NULL && ptrContext->err)
	{
		printf("connect error: %s\n", ptrContext->errstr);
		return;
	}
	//auth
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "auth %s", RedisPWD));
	if (NULL != ptrReply && NULL != ptrReply->str)
	{
		string res(ptrReply->str);
		if ("OK" != res)
		{
			printf("auth error %s\n", ptrReply->str);
			return;
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(ptrReply);
	ptrReply = NULL;
	//select db
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "select %d", 1));
	if (NULL != ptrReply  && REDIS_REPLY_STATUS == ptrReply->type && NULL != ptrReply->str)
	{
		string res(ptrReply->str);
		if ("OK" != res)
		{
			printf("select db error %s\n", ptrReply->str);
			return;
		}
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(ptrReply);
	ptrReply = NULL;
	//subscribe
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, subCommand.c_str()));
	freeReplyObject(ptrReply);
	ptrReply = NULL;

	while (redisGetReply(ptrContext, reinterpret_cast<void**>(&ptrReply)) == REDIS_OK)
	{
		// consume message
		if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 3 == ptrReply->elements) {
			if (strcmp(ptrReply->element[0]->str, "subscribe") != 0) {

				//recv binary data;
				string channelName(ptrReply->element[1]->str, ptrReply->element[1]->len);
				string value(ptrReply->element[2]->str, ptrReply->element[2]->len);

				callback(channelName, value);
			}
		}
		freeReplyObject(ptrReply);
		ptrReply = NULL;
	}
	//unsubscribe
	string unsub("UN");
	unsub += subCommand;
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, unsub.c_str()));
	freeReplyObject(ptrReply);
	ptrReply = NULL;
	//释放redisConnect()所产生的连接
	redisFree(ptrContext);
	ptrContext = NULL;
}

std::string CRedis::Auth(const std::string& password)
{
	std::string str;
	this->m_ptrReply = static_cast<redisReply*>(redisCommand(this->m_ptrContext, "auth %s", password.c_str()));
	if (NULL != m_ptrReply && NULL != m_ptrReply->str)
	{
		str += m_ptrReply->str;
	}
	//释放RedisCommand执行后返回的RedisReply所占用的内存
	freeReplyObject(m_ptrReply);
	m_ptrReply = NULL;
	return str;
}
