#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "MessageChat.pb.h"
#include "Tcp_Session.h"


//设计模式之单件模式 
class ProtobufDispatcher
{
public:
	typedef google::protobuf::Message CMessage;
	typedef std::shared_ptr<CMessage> shared_ptr_Msg;
	//消息回调类型
	typedef std::function<void(const SessionPtr&, const shared_ptr_Msg&)> ProtobufMessageCallback;

	explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCallback)
		: defaultCallback_(defaultCallback_)
	{
	
	};

	~ProtobufDispatcher(){
		google::protobuf::ShutdownProtobufLibrary();
	};
	//注册消息
	void registerMessageCallback(const google::protobuf::Descriptor* pDescriptor, const ProtobufMessageCallback& callback)
	{
		m_mapCallback[pDescriptor] = callback;
	}
	//找到消息类型对应的处理函数
	void HandleMessage(SessionPtr connection, std::shared_ptr<CMessage> ptrMsg) const
	{
		CallbackMap::const_iterator itbegin = m_mapCallback.find(ptrMsg->GetDescriptor());
		if (itbegin != m_mapCallback.end())
		{
			itbegin->second(connection, ptrMsg);
		}
		else
		{
			defaultCallback_(connection, ptrMsg);
		}
	}
private:
	typedef std::map<const google::protobuf::Descriptor*, ProtobufMessageCallback> CallbackMap;
	CallbackMap m_mapCallback;

	ProtobufMessageCallback defaultCallback_;
};


#endif