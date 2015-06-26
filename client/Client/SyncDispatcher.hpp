#ifndef SYNCDISPATCHER_HPP
#define SYNCDISPATCHER_HPP

#include <map>
#include <memory>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "Message.pb.h"

//设计模式之单件模式 
class SyncProtobufDispatcher
{
public:
	typedef google::protobuf::Message CMessage;
	typedef std::shared_ptr<CMessage> shared_ptr_Msg;
	//消息回调类型
	typedef std::function<void(const shared_ptr_Msg&)> ProtobufMessageCallback;
	explicit SyncProtobufDispatcher(const ProtobufMessageCallback& defaultCallback)
		: defaultCallback_(defaultCallback_)
	{

	};

	~SyncProtobufDispatcher(){
/*		google::protobuf::ShutdownProtobufLibrary();*/
	};
	//注册消息
	void registerMessageCallback(const google::protobuf::Descriptor* pDescriptor, const ProtobufMessageCallback& callback)
	{
		m_mapCallback[pDescriptor->full_name()] = callback;
	}
	//找到消息类型对应的处理函数
	void HandleSyncMessage(std::shared_ptr<CMessage> ptrMsg) const
	{
		CallbackMap::const_iterator it = m_mapCallback.find(ptrMsg->GetDescriptor()->full_name());
		if (it != m_mapCallback.end())
		{
			it->second(ptrMsg);
		}
		else
		{
			defaultCallback_(ptrMsg);
		}
	}
private:
	typedef std::map<std::string, ProtobufMessageCallback> CallbackMap;
	CallbackMap m_mapCallback;

	ProtobufMessageCallback defaultCallback_;
};

#endif	//SYNCDISPATCHER_HPP