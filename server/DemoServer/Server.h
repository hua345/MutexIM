#ifndef SERVER_H
#define  SERVER_H
#pragma once
#define WIN32_LEAN_AND_MEAN// make sure all macros are included 
#include <iostream>
#include <mutex>
#include "Server_Info.hpp"
#include "tcp_server.h"
#include "ProtobufDispatcher.hpp"
#include "ProtobufCodec.h"
#include "Redis.h"

using namespace std;

class Server;

//sub/pub��Ϣ
typedef std::function<void(const string&)> SubscribeCallback;
static map<string, SubscribeCallback> g_mapSubscribe;//���ĵ���Ϣ�ʹ�����ӳ��;
static mutex							g_mutex;
class Server
{
public:
	Server(boost::asio::io_service& ioserver, const string& address, const string& port);
	~Server();
public:
	static void ReidsSubscribeSync(const string& subName, const string& value);
private:
	void SubscribeLogin(const string& value);
	void SubscribeUnlogin(const string& value);
	void SubscribePeerMsg(const string& value);
	void SubscribeGroupMsg(const string& value);
	void SubscribeOnline(const string& value);
	void SubscribeOffLine(const string& value);
	//ע�ᶩ�ĵ���Ϣ
	void InitSubscribe();
private:
	//�¼�����
	void InitCallback();
	void onConnection(const SessionPtr& conn);
	void onWriteCompleteCallback(const SessionPtr& conn);
	void onUnknownMessage(const SessionPtr& conn, shared_ptr_Msg message);
	void onShakeHandleReq(const SessionPtr& conn, shared_ptr_Msg message);
	void onMsgData(const SessionPtr& conn, shared_ptr_Msg message);
	//����Peer��Ϣ;
	void doPeerMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//����Group��Ϣ;
	void doOnlineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//����Group publish��Ϣ;
	void doOfflineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//δ����Ϣ����;
	void onUnreadMsgRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//�������δ����Ϣ����;
	int appendPeerMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response);
	int appendGroupMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response);
	//���к�����Ϣ����;
	void onAllUsersInfoRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//���ѷ�����Ϣ����;
	void onFriendsCategoryRequest(const SessionPtr& conn, shared_ptr_Msg message);
	// ����Ⱥ������;
	void onAllGroupRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//Ⱥ��Ա��Ϣ����;
	void onGroupMemberRequest(const SessionPtr& conn, shared_ptr_Msg message);
private:
	//for test
	void TestData();
private:
	boost::asio::ip::tcp::endpoint m_endPoint;//�����˿�
	tcp_server m_tcpserver;			//��������
	ProtobufDispatcher m_dispatcher;//��������
	ProtobufCodec m_codec;			//���л��ͷ����л�
	CRedis		  m_redis;			//redis����
	std::mutex    m_mutex;			//������;

	string        m_strServerSocket;//�ַ�����ʽ�׽���;
};

#endif