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
	//��ȡ��ֵ;
	string Get(const string& key);
	//����ɾ�����ĸ���;
	int DEL(const string& key);
	//��ȡ�����ֵ;
	vector<string> MGet(const vector<string>& vecKey);
	bool Exists(const string& key);
	//��Ӽ�ֵ��;OK success
	string Set(const string& key, const string& value);
	//ѡ�����ݿ�  ; OK success
	string SelectDB(int databaseId);
	//------------------------------------------------map------------------------------------------------------//
	bool HExists(const string& hashMapName, const string& key);
	int HDel(const string& hashMapName, const vector<string>& vecStr);
	int HDel(const string& hashMapName, const string& key);
	//��ȡMAP���ֵ;
	string HGet(const string& hashMapName, const string& key);
	//��ȡMAP��������ֵ;
	map<string, string> HMGet(const string& hashMapName, const vector<string>& vecStr);
	//��ȡ����MAP���ֵ��;
	bool HGetAll(const string& hashMapName, map<string, string>& dataMap);
	//MAP����Ӽ�ֵ��;1 success, 0 failure
	int HSet(const string& hashMapName, const string& key, const string& value);
	//MAP����Ӷ����ֵ��;OK success
	string HMSet(const string& hashMapName, const map<string, string>& mapValue);
	//---------------------------------------------list--------------------------------------------------------//
	//��ȡList��Χֵ;
	vector<string> LRANGE_binary(const string& listName, int begin, int end);
	//----------------------------------------------set----------------------------------------------------------//
	//��ȡ���ϳ�Ա;
	vector<string> SMEMBERS(const string& SetsName);
	//�򼯺���ӳ�Ա;
	int SADD(const string& setName, const string& memberName);
	//�Ƴ����ϳ�Ա;
	int SREM(const string& setName, const string& memberName);
	//------------------------------------------------zset---------------------------------------------------------//
	//�����򼯺����Ԫ��;
	int ZADD(const string& zsetName, int score, const string& memberName);
	//�Ƴ�Ԫ��;
	int ZREM(const string& zsetName, const string& memberName);
	//����Score, socre����Ϊ����Ҳ������Ϊһ������ֵ;����Score;
	string ZINCRBY(const string& zsetName, int changeScore, const string& memberName);
	//ָ�������ڵ����򼯳�Ա���б�;
	vector<string> ZRANGE(const string& zsetName, int begin, int end);
	vector<string> ZREVRANGE(const string& zsetName, int begin, int end);
	//�������� key �У����� score ֵ���� min �� max ֮��(�������� min �� max )�ĳ�Ա�����򼯳�Ա�� score ֵ����(��С����)
	vector<string> ZRANGEBYSCORE(const string& zsetName, int beginScore, int endScore);
	//----------------------------------------------sub/pub------------------------------------------------------//
	//����Ϣ message ���͵�ָ����Ƶ�� channel;���ؽ��յ���Ϣ message �Ķ���������;
	int Publish(const string& channel, const string& content);
	typedef std::function<void(const string&, const string&)> TSubscribeCallback;
	static void Subscribe(const string& subName, TSubscribeCallback);
	//---------------------------------------pipeline---------------------------------------------------------------------------//
	redisContext* GetContext(){ return m_ptrContext; }
private:
	//�������ݿ�;
	bool Connect( string host, int port);
	//��֤����;OK success
	string Auth(const string& password);
private:
	redisContext* m_ptrContext;			//����������ָ��;
	redisReply*	  m_ptrReply;			//�������ָ��;
};

#endif  //_REDIS_H_
