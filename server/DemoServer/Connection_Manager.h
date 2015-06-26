#pragma once
#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H
#include <set>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include "Tcp_Session.h"
#include "Redis.h"
/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class Connection_Manager : private boost::noncopyable
{
public:
	Connection_Manager();
	~Connection_Manager();
	/// Add the specified connection to the manager and start it.
	void start(SessionPtr ptrConnect);

	/// Stop the specified connection.
	void stop(SessionPtr ptrConnect);

	/// Stop all connections.
	void stop_all();

	/// ����û�ID�����ӵ�ӳ��;
	bool userIdMap(const std::string& userId,  SessionPtr ptrConnection);

	/// �鿴�����Ƿ��ڵ�ǰ������;

	SessionPtr findUser(const std::string userId);
private:
	/// The managed connections.
	std::set<SessionPtr> m_sessionSet;//���Ӽ���;

	std::map<std::string, SessionPtr> m_SessionMap;//�û�ID�����ӵ�ӳ��;

	CRedis    m_redis;
};

#endif	//CONNECTION_MANAGER_H
