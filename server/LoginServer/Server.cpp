#include "stdafx.h"
#include "Server.h"
#include <chrono> 

Server::Server(boost::asio::io_service& asyncservice, const string& address, const string& port)
: m_endPoint(boost::asio::ip::address::from_string(address),atoi(port.c_str()))
, m_tcpserver(asyncservice, m_endPoint)
, m_dispatcher(std::bind(&Server::onUnknownMessage, this, std::placeholders::_1, std::placeholders::_2))
, m_codec(std::bind(&ProtobufDispatcher::HandleMessage, &m_dispatcher, std::placeholders::_1, std::placeholders::_2))
{
	InitCallback();
	//建立redis连接;
	m_redis.InitRedis();
	//数据库1是测试数据库;
	m_redis.SelectDB(1);
	//输出服务器信息;
	std::cout << "server run at " << m_endPoint.address().to_string() << " : " << m_endPoint.port() << std::endl;
}


Server::~Server()
{
	
}
void Server::InitCallback()
{
	//注册消息处理函数;
	m_dispatcher.registerMessageCallback(IM::MsgServerRequest::descriptor()
		, std::bind(&Server::onMsgServerRequest, this, std::placeholders::_1, std::placeholders::_2));
	m_dispatcher.registerMessageCallback(IM::LoginRequest::descriptor()
		, std::bind(&Server::onLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	//注册网络连接消息回调函数;
	m_tcpserver.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2));
	m_tcpserver.setConnectedCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
	m_tcpserver.setWriteCompleteCallback(std::bind(&Server::onWriteCompleteCallback, this, std::placeholders::_1));
}

void Server::onConnection(const TSessionPtr& conn)
{
	//do nothing
}

void Server::onWriteCompleteCallback(const TSessionPtr& conn)
{
	//do nothing
}

void Server::onUnknownMessage(const TSessionPtr& conn, shared_ptr_Msg message)
{
	//do nothing
}

void Server::onMsgServerRequest(const TSessionPtr& conn, shared_ptr_Msg message)
{	
	std::shared_ptr<IM::MsgServerRequest> ptrRequest = std::dynamic_pointer_cast<IM::MsgServerRequest>(message);
	IM::MsgServResponse response;
	auto vecRes = m_redis.ZREVRANGE("serverRank", 0, 0);
	if (vecRes.size() == 1)
	{
		cout << ptrRequest->GetTypeName() << ": userId  " << ptrRequest->userid() << endl;

		//对该服务器进行加权;
		m_redis.ZINCRBY("serverRank", -10, vecRes[0]);
		size_t index = vecRes[0].find(':');

		string strAddress(vecRes[0].c_str(), index);
		string strPort(vecRes[0].c_str() + index + 1);
		response.set_result(IM::RESULTERROR::SUCCESS);// 0 -- success, non-0 -- fail reason
		response.set_address(strAddress);
		response.set_port(atoi(strPort.c_str()));
		//save info to database
		string key("loginMap:");
		key += ptrRequest->userid();
		map<string, string> socketData;
		socketData["address"] = strAddress;
		socketData["port"] = strPort;

		m_redis.HMSet(key, socketData);

		m_codec.Send(conn, response);
	}
}

void Server::onLoginRequest(const TSessionPtr& conn, shared_ptr_Msg message)
{
	std::shared_ptr<IM::LoginRequest> ptrRequest = std::dynamic_pointer_cast<IM::LoginRequest>(message);
	
	if (!m_redis.Exists("online:" + ptrRequest->userid()))
	{
		cout << ptrRequest->GetTypeName() << ": userId  " << ptrRequest->userid() <<" MD5 "<<ptrRequest->password()<< endl;

		//get data from databse
		string key("user:");
		key += ptrRequest->userid();
		map<string, string> dataMap;
		m_redis.HGetAll(key, dataMap);
		IM::LoginResponse response;
		time_t nTimeTamp;
		time(&nTimeTamp);
		//check data
		string userID(dataMap["useId"]);
		string password(dataMap["password"]);
		unsigned char md[MD5_DIGEST_LENGTH + 1] = { 0 };//add '\0' for to strcmp
		MD5((const unsigned char*)password.c_str(), password.size(), md);
		if (ptrRequest->userid() == userID && 0 == strcmp((const char*)md, ptrRequest->password().c_str()))
		{
			//获取SessionID并保存到数据库;
			const string sessionID = getSessionID();
			//online list
			string key("loginMap:");
			key += ptrRequest->userid();
			m_redis.HSet(key, "sessionid", sessionID);
			//current login time to database
			char timebuf[20] = { 0 };
			_itoa_s((int32_t)nTimeTamp, timebuf, sizeof(timebuf), 10);
			key.assign("user:");//delete before data
			key += userID;
			map<string, string> hmsetData;
			hmsetData["lastLoginTime"] = timebuf;
			memset(timebuf, 0, 20);
			_itoa_s(ptrRequest->onlinestate(), timebuf, sizeof(timebuf), 10);
			hmsetData["onlineStatus"] = timebuf;
			m_redis.HMSet(key, hmsetData);
			printf("new user has login.userId : %s ,sessionId %s \n", userID.c_str(), sessionID.c_str());
			//publish channel
			m_redis.Publish("login", userID);
			//响应数据;
			response.set_result(IM::RESULTERROR::SUCCESS);// 0 -- success, non-0 -- fail reason
			response.set_timetamp(nTimeTamp);
			response.set_sessionid(sessionID);
			response.set_userid(userID);
			response.set_name(dataMap["username"]);
			response.set_nick_name(dataMap["nickName"]);
			response.set_position(dataMap["position"]);
			response.set_email(dataMap["email"]);
			response.set_image_url(dataMap["imageurl"]);
			response.set_sex(atoi(dataMap["sex"].c_str()));
		}
		else
		{
			string key("loginMap:");
			key += ptrRequest->userid();
			//获取分配服务器信息;
			map<string, string> dataMap;
			m_redis.HGetAll(key, dataMap);
			//删除loginMap信息;
			m_redis.DEL(key);
			//对该服务器进行加权;
			string keySocket(dataMap["address"]);
			keySocket += ':';
			keySocket += dataMap["port"];
			m_redis.ZINCRBY("serverRank", 10, keySocket);
		}

		m_codec.Send(conn, response);
	}
	else
	{
		IM::LoginResponse response;
		response.set_result(IM::RESULTERROR::ONLYONE_ERROR);
		m_codec.Send(conn, response);
	}
}

