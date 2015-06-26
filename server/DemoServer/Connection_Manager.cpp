#include "stdafx.h"
#include "Connection_Manager.h"

Connection_Manager::Connection_Manager()
{
	m_redis.InitRedis();
}

Connection_Manager::~Connection_Manager()
{
	stop_all();
}

void Connection_Manager::start(SessionPtr ptrConnection)
{
	m_sessionSet.insert(ptrConnection);
	ptrConnection->start();
}

void Connection_Manager::stop(SessionPtr ptrConnection)
{
	m_sessionSet.erase(m_sessionSet.find(ptrConnection));

	ptrConnection->stop();

	if (!ptrConnection->get_userId().empty())
	{
		//ɾ��online:userid��ֵ��;
		m_redis.DEL("online:" + ptrConnection->get_userId());
		//�ҵ��û����е�Ⱥ;
		string keyList("groupSet:");
		keyList += ptrConnection->get_userId();
		vector<string> vecStr = m_redis.LRANGE_binary(keyList, 0, -1);
		//�����е�Ⱥ��onlineGroup:groupid�Ƴ���Ա;
		for (size_t i = 0; i < vecStr.size(); i++)
		{
			m_redis.SREM("onlineGroup:" + vecStr[i], ptrConnection->get_userId());
		}
		//publish unlogin��Ϣ;
		m_redis.Publish("offline", ptrConnection->get_userId());
		m_SessionMap.erase(m_SessionMap.find(ptrConnection->get_userId()));
	}
}

void Connection_Manager::stop_all()
{
	std::for_each(m_sessionSet.begin(), m_sessionSet.end(),
		boost::bind(&Tcp_Session::stop, _1));
	m_sessionSet.clear();
	//���redis����;
	auto itBegin = m_SessionMap.begin();
	while (itBegin != m_SessionMap.end())
	{
		//ɾ��online:userid��ֵ�� ,<Key>;
		m_redis.DEL("online:" + itBegin->first);
		//�ҵ��û����е�Ⱥ,<����>;
		string keyList("groupSet:");
		keyList += itBegin->first;
		vector<string> vecStr = m_redis.SMEMBERS(keyList);
		//�����е�Ⱥ��onlineGroup:groupid�Ƴ���Ա;
		for (size_t i = 0; i < vecStr.size(); i++)
		{
			m_redis.SREM("onlineGroup:" + vecStr[i], itBegin->first);
		}
		//publish unlogin��Ϣ;
		m_redis.Publish("offline", itBegin->first);
		//-----------------------------------------------------------------------
		++itBegin;
	}

	//���ӳ���;
	m_SessionMap.clear();
}

bool Connection_Manager::userIdMap(const std::string& userId,  SessionPtr ptrConnection)
{
	bool res = false;
	auto it = m_sessionSet.find(ptrConnection);
	if (it != m_sessionSet.end())
	{
		m_SessionMap[userId] = ptrConnection;
		res = true;
	}
	return res;
}

SessionPtr Connection_Manager::findUser(const std::string userId)
{
	auto it = m_SessionMap.find(userId);
	if (it != m_SessionMap.end())
	{
		return it->second;
	}

	return NULL;
}

