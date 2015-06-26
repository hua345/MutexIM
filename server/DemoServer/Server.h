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

//sub/pub消息
typedef std::function<void(const string&)> SubscribeCallback;
static map<string, SubscribeCallback> g_mapSubscribe;//订阅的消息和处理函数映射;
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
	//注册订阅的消息
	void InitSubscribe();
private:
	//事件处理
	void InitCallback();
	void onConnection(const SessionPtr& conn);
	void onWriteCompleteCallback(const SessionPtr& conn);
	void onUnknownMessage(const SessionPtr& conn, shared_ptr_Msg message);
	void onShakeHandleReq(const SessionPtr& conn, shared_ptr_Msg message);
	void onMsgData(const SessionPtr& conn, shared_ptr_Msg message);
	//处理Peer消息;
	void doPeerMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//处理Group消息;
	void doOnlineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//处理Group publish消息;
	void doOfflineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr);
	//未读消息请求;
	void onUnreadMsgRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//返回添加未读消息个数;
	int appendPeerMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response);
	int appendGroupMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response);
	//所有好友信息请求;
	void onAllUsersInfoRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//好友分组信息请求;
	void onFriendsCategoryRequest(const SessionPtr& conn, shared_ptr_Msg message);
	// 所有群号请求;
	void onAllGroupRequest(const SessionPtr& conn, shared_ptr_Msg message);
	//群成员信息请求;
	void onGroupMemberRequest(const SessionPtr& conn, shared_ptr_Msg message);
private:
	//for test
	void TestData();
private:
	boost::asio::ip::tcp::endpoint m_endPoint;//监听端口
	tcp_server m_tcpserver;			//网络连接
	ProtobufDispatcher m_dispatcher;//反射中心
	ProtobufCodec m_codec;			//序列化和反序列化
	CRedis		  m_redis;			//redis连接
	std::mutex    m_mutex;			//互斥锁;

	string        m_strServerSocket;//字符串形式套接字;
};

#endif