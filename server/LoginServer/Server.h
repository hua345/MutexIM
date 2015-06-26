#ifndef SERVER_H
#define  SERVER_H
#pragma once
#include <iostream>
#include "Server_Info.hpp"
#include "ssl_server.h"
#include "ProtobufDispatcher.hpp"
#include "ProtobufCodec.h"
#include "Redis.h"
#include "RedisSubscribe.h"
#include "SessionId.hpp"
#include <openssl/md5.h>

using namespace std;
class Server
{
public:
	Server(boost::asio::io_service& ioserver, const string& address, const string& port);
	~Server();
private:
	//事件处理
	void InitCallback();
	void onConnection(const TSessionPtr& conn);
	void onWriteCompleteCallback(const TSessionPtr& conn);
	void onUnknownMessage(const TSessionPtr& conn, shared_ptr_Msg message);
	void onMsgServerRequest(const TSessionPtr& conn, shared_ptr_Msg message);
	void onLoginRequest(const TSessionPtr& conn, shared_ptr_Msg message);
private:
	boost::asio::ip::tcp::endpoint m_endPoint;//监听端口
	ssl_server m_tcpserver;			//网络连接
	ProtobufDispatcher m_dispatcher;//反射中心
	ProtobufCodec m_codec;			//序列化和反序列化
	CRedis		  m_redis;			//redis连接
};

#endif