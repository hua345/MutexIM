#ifndef _REDIS_H_
#define _REDIS_H_

#include <ostream>
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <functional>
#include <hiredis/hiredis.h>
#ifdef HIREDIS_WIN
#define snprintf sprintf_s
#endif
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"hiredis.lib")

#define RedisIP		"127.0.0.1"
#define RedisPort    6379
#define RedisPWD    "014006"

using namespace std;
class CRedis
{
public:
	CRedis();

	~CRedis();
	void InitRedis();
	vector<string> SortMultiSocket(const string& groupName);
	//---------------------------------------------------------------------------------------
	//读取键值;
	string Get(const string& key);
	//返回删除键的个数;
	int DEL(const string& key);
	//读取多个键值;
	vector<string> MGet(const vector<string>& vecKey);
	bool Exists(const string& key);
	//添加键值对;OK success
	string Set(const string& key, const string& value);
	//选择数据库  ; OK success
	string SelectDB(int databaseId);
	//------------------------------------------------map------------------------------------------------------//
	bool HExists(const string& hashMapName, const string& key);
	int HDel(const string& hashMapName, const vector<string>& vecStr);
	int HDel(const string& hashMapName, const string& key);
	//读取MAP表键值;
	string HGet(const string& hashMapName, const string& key);
	//读取MAP表多个键的值;
	map<string, string> HMGet(const string& hashMapName, const vector<string>& vecStr);
	//读取整个MAP表键值对;
	bool HGetAll(const string& hashMapName, map<string, string>& dataMap);
	//MAP表添加键值对;1 success, 0 failure
	int HSet(const string& hashMapName, const string& key, const string& value);
	//MAP表添加多个键值对;OK success
	string HMSet(const string& hashMapName, const map<string, string>& mapValue);
	//---------------------------------------------list--------------------------------------------------------//
	//获取List范围值;
	vector<string> LRANGE_binary(const string& listName, int begin, int end);
	//----------------------------------------------set----------------------------------------------------------//
	//获取集合成员;
	vector<string> SMEMBERS(const string& SetsName);
	//向集合添加成员;
	int SADD(const string& setName, const string& memberName);
	//移除集合成员;
	int SREM(const string& setName, const string& memberName);
	//------------------------------------------------zset---------------------------------------------------------//
	//向有序集合添加元素;
	int ZADD(const string& zsetName, int score, const string& memberName);
	//移除元素;
	int ZREM(const string& zsetName, const string& memberName);
	//增加Score, socre可以为正数也可以以为一个负数值;返回Score;
	string ZINCRBY(const string& zsetName, int changeScore, const string& memberName);
	//指定区间内的有序集成员的列表;
	vector<string> ZRANGE(const string& zsetName, int begin, int end);
	vector<string> ZREVRANGE(const string& zsetName, int begin, int end);
	//返回有序集 key 中，所有 score 值介于 min 和 max 之间(包括等于 min 或 max )的成员。有序集成员按 score 值递增(从小到大)
	vector<string> ZRANGEBYSCORE(const string& zsetName, int beginScore, int endScore);
	//----------------------------------------------sub/pub------------------------------------------------------//
	//将信息 message 发送到指定的频道 channel;返回接收到信息 message 的订阅者数量;
	int Publish(const string& channel, const string& content);
	typedef std::function<void(const string&, const string&)> TSubscribeCallback;
	static void Subscribe(const string& subName, TSubscribeCallback);
	//---------------------------------------pipeline---------------------------------------------------------------------------//
	redisContext* GetContext(){ return m_ptrContext; }
private:
	//连接数据库;
	bool Connect( string host, int port);
	//验证密码;OK success
	string Auth(const string& password);
private:
	redisContext* m_ptrContext;			//连接上下文指针;
	redisReply*	  m_ptrReply;			//操作结果指针;
};

#endif  //_REDIS_H_
