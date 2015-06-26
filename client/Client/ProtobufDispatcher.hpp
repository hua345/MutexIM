#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "Message.pb.h"
#include "AsyncTcpClient.h"


//设计模式之单件模式 
class ProtobufDispatcher
{
public:
	typedef google::protobuf::Message CMessage;
	typedef std::shared_ptr<CMessage> shared_ptr_Msg;
	//消息回调类型
	typedef std::function<void(const TcpConnectionPtr&, const shared_ptr_Msg&)> ProtobufMessageCallback;
	explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCallback)
		: defaultCallback_(defaultCallback_)
	{
	
	};

	~ProtobufDispatcher(){
/*		google::protobuf::ShutdownProtobufLibrary();*/
	};
	//注册消息
	void registerMessageCallback(const google::protobuf::Descriptor* pDescriptor, const ProtobufMessageCallback& callback)
	{
		m_mapCallback[pDescriptor->full_name()] = callback;
	}
	//找到消息类型对应的处理函数
	void HandleMessage(TcpConnectionPtr connection, std::shared_ptr<CMessage> ptrMsg) const
	{
		CallbackMap::const_iterator it = m_mapCallback.find(ptrMsg->GetDescriptor()->full_name());
		if (it != m_mapCallback.end())
		{
			it->second(connection, ptrMsg);
		}
		else
		{
			defaultCallback_(connection, ptrMsg);
		}
	}
private:
	typedef std::map<std::string , ProtobufMessageCallback> CallbackMap;
	CallbackMap m_mapCallback;

	ProtobufMessageCallback defaultCallback_;
};


#endif