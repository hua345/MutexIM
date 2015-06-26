#include "stdafx.h"
#include "Server.h"
#include <thread>
#include <unordered_map>
//#include "AsyncRedis.hpp"
#include "MessageServer.pb.h"

Server::Server(boost::asio::io_service& asyncservice, const string& address, const string& port)
: m_endPoint(boost::asio::ip::address::from_string(address), atoi(port.c_str()))
, m_tcpserver(asyncservice, m_endPoint)
, m_dispatcher(std::bind(&Server::onUnknownMessage, this, std::placeholders::_1, std::placeholders::_2))
, m_codec(std::bind(&ProtobufDispatcher::HandleMessage, &m_dispatcher, std::placeholders::_1, std::placeholders::_2))
{
	m_mutex.lock();

	InitCallback();
	InitSubscribe();
	//����redis����
	m_redis.InitRedis();

	//�����������Ϣ
	std::cout << "server run at " << m_endPoint.address().to_string()<< " : " <<m_endPoint.port() << std::endl;
	//��redisע�������;
	m_strServerSocket = m_endPoint.address().to_string();
	m_strServerSocket += ':';
	char buf[10] = { 0 };
	_itoa_s(m_endPoint.port(), buf, sizeof(buf), 10);
	m_strServerSocket += buf;
	m_redis.ZADD("serverRank", 100, m_strServerSocket);

	//������Ϣ;
	string subscribeCommand("SUBSCRIBE");
	auto itBegin = g_mapSubscribe.begin();
	while (itBegin != g_mapSubscribe.end())
	{
		subscribeCommand += ' ';
		subscribeCommand += itBegin->first;
		++itBegin;
	}
	std::cout << subscribeCommand << std::endl;
	std::thread subscribeThread(&CRedis::Subscribe, subscribeCommand.c_str(), ReidsSubscribeSync);
	subscribeThread.detach();
	m_mutex.unlock();
}

Server::~Server()
{
	m_redis.ZREM("serverRank", m_strServerSocket);
	
	m_tcpserver.getConnection_Manager().stop_all();
}

void Server::SubscribeLogin(const string& value)
{
	cout << "login :" << value << endl;
}

void Server::SubscribeUnlogin(const string& value)
{
	cout << "unLogin :" << value << endl;
}

void Server::SubscribePeerMsg(const string& value)
{
	m_mutex.lock();
	IMChat::MsgData  msg;
	msg.ParseFromString(value);
	//�ҵ��û�������;
	SessionPtr ptrConnection = m_tcpserver.getConnection_Manager().findUser(msg.toid());
	if (NULL != ptrConnection)
	{
		cout << "peerMsg:" << m_strServerSocket <<" : toId  "<< msg.toid() << "  fromId  "<<msg.fromid()<<" Value "<<msg.msgdata()<< endl;
		m_codec.Send(ptrConnection, msg);
	}
	m_mutex.unlock();
}

void Server::SubscribeGroupMsg(const string& value)
{
	m_mutex.lock();
	IMServer::serverGroupMsg msg;
	msg.ParseFromString(value);
	
	IMChat::MsgData contentData;
	contentData.ParseFromString(msg.msgdata());
	cout << "groupMsg:" << m_strServerSocket;
	for (int i = 0; i < msg.userid_size(); i++)
	{
		//�ҵ��û�������;
		SessionPtr ptrConnection = m_tcpserver.getConnection_Manager().findUser(msg.userid(i));
		cout<<"   " << msg.userid(i);
		if (NULL != ptrConnection)
		{
			m_codec.Send(ptrConnection, contentData);
		}
	}
	cout << endl;
	m_mutex.unlock();
}


void Server::SubscribeOnline(const string& value)
{
	cout << "online :" << value << endl;
}

void Server::SubscribeOffLine(const string& value)
{
	cout << "offline :" << value << endl;
}

void Server::InitSubscribe()
{
	g_mutex.lock();
	//ע�ᶩ�ĵ���Ϣ;
	string strSocket(m_endPoint.address().to_string());
	char buf[20] = { 0 };
	_itoa_s(m_endPoint.port(), buf, 10);
	strSocket += ':';
	strSocket += buf;
	g_mapSubscribe["login"] = std::bind(&Server::SubscribeLogin, this, std::placeholders::_1);
	g_mapSubscribe["unlogin"] = std::bind(&Server::SubscribeUnlogin, this, std::placeholders::_1);
	g_mapSubscribe["online"] = std::bind(&Server::SubscribeOnline, this, std::placeholders::_1);
	g_mapSubscribe["offline"] = std::bind(&Server::SubscribeOffLine, this, std::placeholders::_1);
	g_mapSubscribe["peerMsg:" + strSocket] = std::bind(&Server::SubscribePeerMsg, this, std::placeholders::_1);
	g_mapSubscribe["groupMsg:" + strSocket] = std::bind(&Server::SubscribeGroupMsg, this, std::placeholders::_1);

	g_mutex.unlock();
}

void Server::InitCallback()
{
	//ע����Ϣ������;
	//������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::MsgData::descriptor()
		, std::bind(&Server::onMsgData, this, std::placeholders::_1, std::placeholders::_2));
	//δ��������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::UnreadMsgRequest::descriptor()
		, std::bind(&Server::onUnreadMsgRequest, this, std::placeholders::_1, std::placeholders::_2));
	//����������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::shakeHandleReq::descriptor()
		, std::bind(&Server::onShakeHandleReq, this, std::placeholders::_1, std::placeholders::_2));
	//������Ϣ������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::PeerAllUserInfoRequest::descriptor()
		, std::bind(&Server::onAllUsersInfoRequest, this, std::placeholders::_1, std::placeholders::_2));
	//Ⱥ��Ϣ������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::GroupAllRequest::descriptor()
		, std::bind(&Server::onAllGroupRequest, this, std::placeholders::_1, std::placeholders::_2));
	//Ⱥ��Ա������Ϣ;
	m_dispatcher.registerMessageCallback(IMChat::GroupMembeRequest::descriptor()
		, std::bind(&Server::onGroupMemberRequest, this, std::placeholders::_1, std::placeholders::_2));
	//ע������������Ϣ�ص�����;
	m_tcpserver.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2));
	m_tcpserver.setConnectedCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
	m_tcpserver.setWriteCompleteCallback(std::bind(&Server::onWriteCompleteCallback, this, std::placeholders::_1));
}

void Server::onConnection(const SessionPtr& conn)
{
	//do nothing
}

void Server::onWriteCompleteCallback(const SessionPtr& conn)
{
	//do nothing
}

void Server::onUnknownMessage(const SessionPtr& conn, shared_ptr_Msg message)
{
	//do nothing
}

void Server::onShakeHandleReq(const SessionPtr& conn, shared_ptr_Msg message)
{
	//�õ����������˭;
	std::shared_ptr<IMChat::shakeHandleReq> ptrRequest = std::dynamic_pointer_cast<IMChat::shakeHandleReq>(message);
	if (NULL != ptrRequest)
	{
		cout << ptrRequest->GetTypeName() << ":" << ptrRequest->userid() << endl;
		map<string, string> dataMap;
		string key("loginMap:");
		key += ptrRequest->userid();
		m_redis.HGetAll(key, dataMap);
		if (dataMap.size() != 0) //����û���½��Map��Ϊ��;
		{
			if (!m_redis.Exists("online:" + ptrRequest->userid()))
			{
				//����������sessionId��userId��
				conn->set_SessionId(dataMap["sessionid"]);
				conn->set_userId(ptrRequest->userid());
				//�����ӹ���������Ϣ;
				m_tcpserver.getConnection_Manager().userIdMap(ptrRequest->userid(), conn);
				//���online:userid��ֵ��;
				m_redis.Set("online:" + ptrRequest->userid(), m_strServerSocket);
				//�ҵ��û����е�Ⱥ;
				string keySet("groupSet:");
				keySet += ptrRequest->userid();
				vector<string> vecStr = m_redis.SMEMBERS(keySet);
				//�����е�Ⱥ��onlineGroup:groupid��ӳ�Ա;
				for (size_t i = 0; i < vecStr.size(); i++)
				{
					m_redis.SADD("onlineGroup:" + vecStr[i], ptrRequest->userid());
				}
				//publish online ��Ϣ;
				m_redis.Publish("online", ptrRequest->userid());
				//���ؽ��;
				IMChat::shakeHandleRes response;
				time_t nowtime;
				std::time(&nowtime);
				response.set_timestamp(nowtime);
				response.set_result(0);
				m_codec.Send(conn, response);
			}
		}
		else
		{
			//���ؽ��;
			IMChat::shakeHandleRes response;
			response.set_result(2);
			time_t nowtime;
			std::time(&nowtime);
			response.set_timestamp(nowtime);
			response.set_result(0);
			m_codec.Send(conn, response);
		}
	}
}
//������Ϣ;
void Server::onMsgData(const SessionPtr& conn, shared_ptr_Msg message)
{

	std::shared_ptr<IMChat::MsgData> ptrMsg = std::dynamic_pointer_cast<IMChat::MsgData>(message);
	if (NULL != ptrMsg)
	{
		cout << ptrMsg->GetTypeName() <<" : type"<<ptrMsg->type()<< " fromId  " << ptrMsg->fromid() <<"  toId  "<<ptrMsg->toid()<<" value "<<ptrMsg->msgdata()<< endl;
		//��Ϣ���л��ַ���;
		string serialStr = ptrMsg->SerializeAsString();
		//��Ե���Ϣ;
		if (IMChat::MsgData_MSGTYPE_CHAT_TEXT == ptrMsg->type() || IMChat::MsgData_MSGTYPE_CHAT_AUDIO == ptrMsg->type())
		{
			doPeerMsg(ptrMsg, serialStr);
		}
		//Ⱥ��Ϣ;
		else if (IMChat::MsgData_MSGTYPE_GROUPCHAT_TEXT == ptrMsg->type() || IMChat::MsgData_MSGTYPE_GROUPCHAT_AUDIO == ptrMsg->type())
		{
			doOnlineGroupMsg(ptrMsg, serialStr);

			doOfflineGroupMsg(ptrMsg, serialStr);
		}
	}
}

void Server::doPeerMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr)
{
	m_mutex.lock();
	//��������;
	SessionPtr ptrConnection = m_tcpserver.getConnection_Manager().findUser(ptrMsg->toid());
	if (NULL != ptrConnection)
	{
		//����û��ڵ�ǰ������ֱ�ӷ���;
		m_codec.Send(ptrConnection, *ptrMsg);
	}
	else
	{
		string serverSocket = m_redis.Get("online:" + ptrMsg->toid());
		//����û�������������Publish��Ϣ;
		if (!serverSocket.empty())
		{
			string keyServer("peerMsg:");
			keyServer += serverSocket;
			m_redis.Publish(keyServer, serializaStr);
		}
		else
		{
			//����û�δ��½;����Ϣ���뵽δ����Ϣ�б�;	
			//δ����Ϣ����;
			int nListLen = 0;
			auto ptrContext = m_redis.GetContext();
			redisReply* ptrReply = NULL;

			string keyUnreadList("unreadMsg:");
			keyUnreadList += ptrMsg->toid();
			//��������б�;
			ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LPUSH %s %b", keyUnreadList.c_str(), serializaStr.c_str(), serializaStr.size()));
			if (NULL != ptrReply && REDIS_REPLY_INTEGER == ptrReply->type)
			{
				nListLen = ptrReply->integer;
			}
			freeReplyObject(ptrReply);
			//ֻ����99����Ϣ;���������ȡ����99��;
			if (nListLen > 99)
			{
				ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LTRIM %s %d %d", keyUnreadList.c_str(), 0, 99));
				freeReplyObject(ptrReply);
			}
		}
	}
	m_mutex.unlock();
}

void Server::doOnlineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg, const string& serializaStr)
{
	m_mutex.lock();
	//�����з�����Publish��Ϣ,���������ݿ�;
	string keyOnlineGroup("onlineGroup:");

	keyOnlineGroup += ptrMsg->toid();
	//vector ����������Ϊ��һ��userid�ڶ���serverSocket....;
	vector<string> vecStr = m_redis.SortMultiSocket(keyOnlineGroup);
	//�õ�����������Socket����;
	//first->socket second-> userid
	unordered_map<string, string> dataUnorderMap;
	for (int i = 0; i < vecStr.size() / 2; i++)
	{
		if (vecStr[2 * i + 1] == m_strServerSocket)
		{
			if (ptrMsg->fromid() != vecStr[2 * i])
			{
				//�û��ڵ�ǰ��������;
				SessionPtr ptrConnection = m_tcpserver.getConnection_Manager().findUser(vecStr[2 * i]);
				if (NULL != ptrConnection)
				{
					m_codec.Send(ptrConnection, *ptrMsg);
				}
			}
		}
		else
		{
			dataUnorderMap[vecStr[2 * i + 1]] = vecStr[2 * i];
		}
	}
	//������������Socket����Publish����;
	if (dataUnorderMap.size() >= 1)
	{
		for (size_t i = 0; i < dataUnorderMap.bucket_count(); ++i)
		{
			IMServer::serverGroupMsg  publishData;
			bool bFlag = false;
			//socket -> userid
			for (auto local_it = dataUnorderMap.begin(i); local_it != dataUnorderMap.end(i); ++local_it)
			{
				publishData.add_userid()->assign(local_it->second);
				bFlag = true;
			}
			if (true == bFlag)
			{
				publishData.set_msgdata(serializaStr);
				//publish��Ϣ;
				string channelName("groupMsg:");
				channelName += dataUnorderMap.begin(i)->first;
				m_redis.Publish(channelName, publishData.SerializeAsString());
			}
		}
	}
	m_mutex.unlock();
}

void Server::doOfflineGroupMsg(shared_ptr<IMChat::MsgData> ptrMsg , const string& serializaStr)
{
	m_mutex.lock();
	//δ����Ϣ����;
	int nGroupMsgLen = 0;
	auto ptrContext = m_redis.GetContext();
	redisReply* ptrReply = NULL;

	string keyGroupMsgList("groupMsg:");
	keyGroupMsgList += ptrMsg->toid();
	//��������б�洢����������;
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LPUSH %s %b", keyGroupMsgList.c_str(), serializaStr.c_str(), serializaStr.size()));
	if (NULL != ptrReply && REDIS_REPLY_INTEGER == ptrReply->type)
	{
		nGroupMsgLen = ptrReply->integer;
	}
	freeReplyObject(ptrReply);
	//ֻ����99����Ϣ;���������ȡ����99��;
	if (nGroupMsgLen > 99)
	{
		ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LTRIM %s %d %d", keyGroupMsgList.c_str(), 0, 99));
		freeReplyObject(ptrReply);
	}
	//����û�����ߵ��û�, Key unread:groupid:userid   +1;
	string onlineKey("onlineGroup:");
	string groupMemberKey("groupMember:");
	onlineKey += ptrMsg->toid();
	groupMemberKey += ptrMsg->toid();
	//����һ�����ϵ�ȫ����Ա���ü��������и�������֮��Ĳ��;��һ���������������ϵĲ;
	vector<string> vecRes;
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "SDIFF %s %s", groupMemberKey.c_str(), onlineKey.c_str()));
	if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
	{
		for (size_t i = 0; i < ptrReply->elements; i++)
			vecRes.push_back(ptrReply->element[i]->str);
	}
	freeReplyObject(ptrReply);
	//pipeline
	//Key unreadNumber : userid : groupid ;+ 1;�����ѯ����;
	for (size_t i = 0; i < vecRes.size(); i++)
	{
		string key("unreadNumber:");
		key += vecRes[i];
		key += ':';
		key += ptrMsg->toid();

		redisAppendCommand(ptrContext, "INCR %s", key.c_str());
	}

	for (size_t i = 0; i < vecRes.size(); i++)
	{
		redisGetReply(ptrContext, reinterpret_cast<void**>(&ptrReply)); // reply for SET
		freeReplyObject(ptrReply);
	}
	m_mutex.unlock();
}

void Server::onUnreadMsgRequest(const SessionPtr& conn, shared_ptr_Msg message)
{
	//binary data;
	m_mutex.lock();
	std::shared_ptr<IMChat::UnreadMsgRequest> ptrRequest = std::dynamic_pointer_cast<IMChat::UnreadMsgRequest>(message);
	if (NULL != ptrRequest)
	{
		cout << ptrRequest->GetTypeName() << ": userId  " <<ptrRequest->userid()<< endl;
		IMChat::UnreadMsgResponse  response;
		//���δ����Ϣ;
		int nMsgNumber = appendPeerMsg(ptrRequest, response);
		nMsgNumber += appendGroupMsg(ptrRequest, response);
		//����repeated�ֶ����ݲ���Ϊ��;
		if (nMsgNumber >= 1)
		{
			response.set_msgcount(nMsgNumber);

			response.set_userid(ptrRequest->userid());
			time_t nowtime;
			std::time(&nowtime);
			response.set_timestamp(static_cast<int64_t>(nowtime));
			m_codec.Send(conn, response);
		}
	}
	m_mutex.unlock();
}

int Server::appendPeerMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response)
{
	string key("unreadMsg:");
	key += ptrRequest->userid();

	auto unreadMsg = m_redis.LRANGE_binary(key, 0, -1);

	m_redis.DEL(key);

	if (unreadMsg.size() != 0)
	{
		for (size_t i = 0; i < unreadMsg.size(); ++i)
		{
			//���repeated����;
			response.add_msgdata()->assign(unreadMsg[i].c_str(), unreadMsg[i].size());
		}
	}
	return unreadMsg.size();
}

int Server::appendGroupMsg(shared_ptr<IMChat::UnreadMsgRequest> ptrRequest, IMChat::UnreadMsgResponse&  response)
{
	auto ptrContext = m_redis.GetContext();
	redisReply* ptrReply = NULL;

	string keyGroupSet("groupSet:");
	keyGroupSet += ptrRequest->userid();

	string keyNumber("unreadNumber:");
	keyNumber += ptrRequest->userid();

	keyNumber += ":*";

	//��ѯ����Ⱥδ����Ϣ��Ŀ;
	//COMMAND: SORT groupSet:14006 GET # GET unreadNumber:14006:*

	vector<string> numberData;
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "SORT %s GET # GET %s", keyGroupSet.c_str(), keyNumber.c_str()));
	if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
	{
		for (size_t i = 0; i < ptrReply->elements; i++)
		{
			numberData.push_back(string(ptrReply->element[i]->str, ptrReply->element[i]->len));
		}
	}
	freeReplyObject(ptrReply);
	for (int i = 0; i < numberData.size()/2; i++)
	{
		if (numberData[2 * i + 1].compare("0") != 0)
		{
			//vector ���ݵ�һ����groupId�ڶ�����δ��Ⱥ��Ϣ��Ŀ;
			string listName("groupMsg:");
			listName += numberData[2 * i];
			redisAppendCommand(ptrContext, "LRANGE %s %d %s", listName.c_str(), 0, numberData[2 * i + 1]);
		}
	}
	//δ����Ϣ��;
	int nMsgNumber = 0;
	for (size_t i = 0; i < numberData.size()/2; i++)
	{
		if (numberData[2 * i + 1].compare("0") != 0)
		{
			redisGetReply(ptrContext, reinterpret_cast<void**>(&ptrReply));
			if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
			{
				for (size_t i = 0; i < ptrReply->elements; i++)
				{
					//���repeated����;
					response.add_msgdata()->assign(ptrReply->element[i]->str, ptrReply->element[i]->len);
				}
				nMsgNumber += ptrReply->elements;
			}
			freeReplyObject(ptrReply);
		}
	}
	//��δ����Ϣ��Ŀ��0;
	for (size_t i = 0; i < numberData.size() / 2; i++)
	{
		if (numberData[2 * i + 1].compare("0") != 0)
		{
			//Key unreadNumber:userid:groupid
			string key("unreadNumber:");
			key += ptrRequest->userid();
			key += ':';
			key += numberData[2 * i];
			m_redis.Set(key, "0");
		}
	}
	return nMsgNumber;
}
void Server::onAllUsersInfoRequest(const SessionPtr& conn, shared_ptr_Msg message)
{
	m_mutex.lock();
	std::shared_ptr<IMChat::PeerAllUserInfoRequest> ptrRequest = std::dynamic_pointer_cast<IMChat::PeerAllUserInfoRequest>(message);
	if (NULL != ptrRequest)
	{
		cout << ptrRequest->GetTypeName() << ": userId  " << ptrRequest->userid() << endl;
		string key("friendsSet:");
		key += ptrRequest->userid();
		auto vecFriends = m_redis.SMEMBERS(key);
		if (vecFriends.size() != 0)
		{
			IMChat::PeerAllUserInfoResponse  response;
			//pipeline
			auto ptrContext = m_redis.GetContext();
			redisReply *ptrReply;
			for (auto& x : vecFriends)
			{
				string userKey("user:");
				userKey += x;
				redisAppendCommand(ptrContext, "HGETALL %s", userKey.c_str());
			}
			for (size_t i = 0; i < vecFriends.size(); i++)
			{
				//��ȡ�û���Ϣ;
				redisGetReply(ptrContext, reinterpret_cast<void**>(&ptrReply)); // reply for userinfo
				if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
				{
					// ���һ���������ݳ�Ա;//repeated����;
					IMChat::UserData* ptrUserData = response.add_msgdata();
					//�Ե�ǰ����ֵ;
					for (size_t i = 0; i < ptrReply->elements /2; ++i) {
						//key
						string field(ptrReply->element[2 * i]->str);
						//value
						if ("useId" == field)
							ptrUserData->set_user_id(ptrReply->element[2 * i + 1]->str);
						else if ("username" == field)
							ptrUserData->set_name(ptrReply->element[2 * i + 1]->str);
						else if ("nickName" == field)
							ptrUserData->set_nick_name(ptrReply->element[2 * i + 1]->str);
						else if ("imageurl" == field)
							ptrUserData->set_image_url(ptrReply->element[2 * i + 1]->str);
						else if ("email" == field)
							ptrUserData->set_email(ptrReply->element[2 * i + 1]->str);
						else if ("sex" == field)
							ptrUserData->set_sex(atoi(ptrReply->element[2 * i + 1]->str));
						else if ("onlineStatus" == field)
							ptrUserData->set_online_status(atoi(ptrReply->element[2 * i + 1]->str));
						else if ("position" == field)
							ptrUserData->set_position(ptrReply->element[2 * i + 1]->str);
					}
					ptrUserData = NULL;
				}
				freeReplyObject(ptrReply);
			}
			response.set_friendscount(vecFriends.size());
			response.set_userid(ptrRequest->userid());
			time_t nowtime;
			time(&nowtime);
			response.set_timestamp(static_cast<int64_t>(nowtime));
			m_codec.Send(conn, response);
		}
		
	}
	m_mutex.unlock();
}

void Server::onFriendsCategoryRequest(const SessionPtr& conn, shared_ptr_Msg message)
{
	//do nothing
}

void Server::onAllGroupRequest(const SessionPtr& conn, shared_ptr_Msg message)
{
	m_mutex.lock();
	std::shared_ptr<IMChat::GroupAllRequest> ptrRequest = std::dynamic_pointer_cast<IMChat::GroupAllRequest>(message);
	if (NULL != ptrRequest)
	{
		cout << ptrRequest->GetTypeName() << ": userId  " << ptrRequest->userid() << endl;
		string key("groupSet:");
		key += ptrRequest->userid();
		auto vecGroups = m_redis.SMEMBERS(key);
		if (vecGroups.size() != 0)
		{
			IMChat::GroupAllResponse  response;
			//pipeline
			auto ptrContext = m_redis.GetContext();
			redisReply *ptrReply;
			for (auto& x : vecGroups)
			{
				string userKey("group:");
				userKey += x;
				redisAppendCommand(ptrContext, "HGETALL %s", userKey.c_str());
			}
			for (size_t i = 0; i < vecGroups.size(); i++)
			{
				//��ȡ�û���Ϣ;
				redisGetReply(ptrContext, reinterpret_cast<void**>(&ptrReply)); // reply for userinfo
				if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
				{
					// ���һ���������ݳ�Ա;//repeated����;
					IMChat::GroupInfo*  ptrGroupInfo = response.add_msgdata();
					//�Ե�ǰ����ֵ;
					for (size_t i = 0; i < ptrReply->elements/2; ++i) {
						//key
						string field(ptrReply->element[2 * i]->str);
						//value
						if ("groupId" == field)
							ptrGroupInfo->set_groupid(ptrReply->element[2 * i + 1]->str);
						else if ("groupName" == field)
							ptrGroupInfo->set_groupname(ptrReply->element[2 * i + 1]->str);
						else if ("imageUrl" == field)
							ptrGroupInfo->set_image_url(ptrReply->element[2 * i + 1]->str);

					}

					ptrGroupInfo = NULL;
				}
				freeReplyObject(ptrReply);
			}
			response.set_groupcount(vecGroups.size());
			response.set_userid(ptrRequest->userid());
			time_t nowtime;
			time(&nowtime);
			response.set_timestamp(static_cast<int64_t>(nowtime));
			int i = response.msgdata_size();
			auto aa = response.msgdata(1);
			m_codec.Send(conn, response);
		}

	}
	m_mutex.unlock();
}

void Server::onGroupMemberRequest(const SessionPtr& conn, shared_ptr_Msg message)
{
	m_mutex.lock();
	std::shared_ptr<IMChat::GroupMembeRequest> ptrRequest = std::dynamic_pointer_cast<IMChat::GroupMembeRequest>(message);
	if (NULL != ptrRequest)
	{
		cout << ptrRequest->GetTypeName() << ": userId  " << ptrRequest->userid() << endl;
		auto ptrContext = m_redis.GetContext();
		redisReply* ptrReply = NULL;
		//��Ӧ;
		IMChat::GroupMemberResponse response;
		string keyMember("groupMember:");
		keyMember += ptrRequest->groupid();
		ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "SORT %s GET # GET user:*->nickName GET user:*->imageurl GET online:* GET user:*->onlineStatus", keyMember.c_str()));
		int nFieldLen = 5;
		if (NULL != ptrReply && REDIS_REPLY_ARRAY == ptrReply->type && 0 != ptrReply->elements)
		{			
			for (size_t i = 0; i < ptrReply->elements /nFieldLen; i++)
			{
				IMChat::GroupMemberInfo*  ptrInfo = response.add_msgdata();
				ptrInfo->set_userid(ptrReply->element[nFieldLen * i]->str);
				ptrInfo->set_nick_name(ptrReply->element[nFieldLen * i + 1]->str);
				ptrInfo->set_image_url(ptrReply->element[nFieldLen * i + 2]->str);
				//�ж��Ƿ�����;
				if (0 != ptrReply->element[nFieldLen * i + 3]->len)
				{
					int32_t status = atoi(ptrReply->element[nFieldLen * i + 4]->str);
					ptrInfo->set_online_status(status);
				}
				else
					ptrInfo->set_online_status(-1);		
			}
			response.set_membercount(ptrReply->elements / nFieldLen);

		}
		freeReplyObject(ptrReply);
		response.set_groupid(ptrRequest->groupid());
		response.set_userid(ptrRequest->userid());
		time_t nowtime;
		std::time(&nowtime);
		response.set_timestamp(static_cast<int64_t>(nowtime));

		m_codec.Send(conn, response);
	}
	m_mutex.unlock();
}

void Server::TestData()
{
	string serialStr;
	IMChat::MsgData msg;
	msg.set_fromid("14007");
	msg.set_toid("14010");
	msg.set_type(IMChat::MsgData_MSGTYPE::MsgData_MSGTYPE_CHAT_TEXT);
	msg.set_sequencenumber(1);
	msg.set_msgdata("this a test data");

	serialStr = msg.SerializeAsString();

	m_redis.Publish("binary", serialStr);
//-------------------------------------------------------------------
	auto ptrContext = m_redis.GetContext();
	redisReply* ptrReply = NULL;
	//��������;
// 	auto vec = m_redis.LRANGE("unreadMsg:14006", 0, -1);
// 	IMChat::MsgData m1, m2;
// 
// 	m1.ParseFromArray(vec[0].c_str(), vec[0].size());
// 	m2.ParseFromArray(vec[1].c_str(), vec[1].size());
	//�������;

	//store binary data
	//��Ҫ���Ͷ����ư�ȫ��������Բ���%b�ĸ�ʽ����ʽ;

	string key("unreadMsg:");
	key += "14006";
	msg.set_sequencenumber(3);
	msg.set_fromid("14006");
	msg.set_msgdata("this a test data3");
	serialStr = msg.SerializeAsString();
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LPUSH %s %b", key.c_str(), serialStr.c_str(), serialStr.size()));

	freeReplyObject(ptrReply);

	msg.set_fromid("14006");
	msg.set_msgdata("this a test data2");

	serialStr = msg.SerializeAsString();
	//store binary data
	//��Ҫ���Ͷ����ư�ȫ��������Բ���%b�ĸ�ʽ����ʽ;
	ptrReply = static_cast<redisReply*>(redisCommand(ptrContext, "LPUSH unreadMsg:14010 %b", serialStr.c_str(), serialStr.size()));
	freeReplyObject(ptrReply);
}

void Server::ReidsSubscribeSync(const string& subName, const string& value)
{
	g_mutex.lock();
	string strSubName(subName);
	auto it = g_mapSubscribe.find(subName);
	if (it != g_mapSubscribe.end())
	{
		it->second(value);
	}

	g_mutex.unlock();
}
