#include "stdafx.h"
#include "ProtobufCodec.h"

// #if !defined(UNDER_CE) && defined(_DEBUG)
// #define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
// #endif

ProtobufCodec::ProtobufCodec(const ProtobufMessageCallback& callback)
: messageCallback_(callback)// syncMessageCallback_(NULL)
{

}
ProtobufCodec::ProtobufCodec(const SyncMessageCallback& callback, bool flag)
:syncMessageCallback_(callback)// messageCallback_(NULL)
{

}
ProtobufCodec::~ProtobufCodec()
{
	/*		google::protobuf::ShutdownProtobufLibrary();*/
}

shared_ptr_Msg ProtobufCodec::CreateMessage(const std::string& type_name)
{
	shared_ptr_Msg message = NULL;
	const google::protobuf::Descriptor* descriptor =
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
	if (descriptor)
	{
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
		{
			//������ָ���������
			message.reset(prototype->New());
		}
	}
	return message;
};
// +--------------------------------------------------------------+     
// |                       message nameLen                        |     uint32
// +--------------------------------------------------------------+
// |                                                              |
// |                          message  name                       |
// |                                                              |
// +--------------------------------------------------------------+
// |                                                              |
// |                             data                             |		 
// |                                                              |
// +--------------------------------------------------------------+ 
// |							checkSum                          |     uint32 // adler32 of nameLen, typeName and protobufData
// +--------------------------------------------------------------+	
shared_ptr_Msg ProtobufCodec::Decode(const char* buf, uint32_t len)
{
	//len is length of the namelength, name,data,checksum 
	shared_ptr_Msg result = NULL;
	if (IsCorrect(buf, len))
	{
		//get message type name
		uint32_t nameLen = PeekUint32(buf);
		if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
		{
			std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
			//create message object
			shared_ptr_Msg message = CreateMessage(typeName);
			if (message)
			{
				//parse from buffer
				const char* data = buf + kHeaderLen + nameLen;
				int32_t dataLen = len - nameLen - 2 * kHeaderLen;//namelen,name,checksum
				if (message->ParseFromArray(data, dataLen))
				{
					result = message;
				}
				else
				{
					message->Clear();
				}
			}
			else
			{
				// unknown message type
			}
		}
		else
		{
			// invalid name len
		}
	}
	else
	{
		// check sum error
	}
	return result;
}

std::shared_ptr<MyBuffer> ProtobufCodec::Encode(const CMessage& message)
{
	std::shared_ptr<MyBuffer> ptrBuffer(new MyBuffer());
	assert(0 == ptrBuffer->readableBytes());
	const std::string& typeName = message.GetTypeName();

	int32_t nameLen = static_cast<int32_t>(typeName.size() + 1);//'\0'

	ptrBuffer->appendUint32(nameLen);
	ptrBuffer->append(typeName.c_str(), nameLen);
	//��Ϣ����
	size_t byte_size = message.ByteSize();
	//ensure memory enough
	ptrBuffer->ensureWritableBytes(byte_size);
	//write data
	message.SerializeWithCachedSizesToArray(reinterpret_cast<uint8_t*>(ptrBuffer->beginWrite()));
	ptrBuffer->commit(byte_size);//move write index

	//�����
	uint32_t checkSum = adler32(1, reinterpret_cast<const Bytef*>(ptrBuffer->peek()), ptrBuffer->readableBytes());

	ptrBuffer->appendUint32(checkSum);
	assert(ptrBuffer->readableBytes() == sizeof(nameLen)+nameLen + byte_size + sizeof(checkSum));
	//��Ϣ����
	int32_t len = htonl(static_cast<int32_t>(ptrBuffer->readableBytes()));
	ptrBuffer->prepend(&len, sizeof(len));
	//delimiter
	ptrBuffer->append("\r\n", 2);

//  for test
// 	std::string strmessage(ptrBuffer->peek() + kHeaderLen, ptrBuffer->readableBytes());
// 	const uint32_t aa = PeekUint32(ptrBuffer->peek());//after ntohl 
// 	shared_ptr_Msg dd = Decode(strmessage.c_str() , aa);//checksum and "\r\n"
// 	const uint32_t bb = PeekUint32(strmessage.c_str()  + aa - kHeaderLen);//��ȡ�����
// 	const uint32_t cc = adler32(1, reinterpret_cast<const Bytef*>(strmessage.c_str()), aa - kHeaderLen);
	return ptrBuffer;
}

uint32_t ProtobufCodec::PeekUint32(const char* buf)
{
	uint32_t be32 = 0;
	int i = sizeof(be32);
	memcpy(&be32, buf, sizeof(be32));
	return ntohl(be32);
}




bool ProtobufCodec::IsCorrect(const char* buf, int32_t len)
{
	const uint32_t expectedCheckSum = PeekUint32(buf + len - kHeaderLen);//��ȡ�����
	const uint32_t checkSum = adler32(1, reinterpret_cast<const Bytef*>(buf), len - kHeaderLen);
	if (checkSum == expectedCheckSum)
		return true;
	return false;
}

void ProtobufCodec::onMessage(const TcpConnectionPtr& connection, const std::string& strmessage)
{
	//message length
	const uint32_t len = PeekUint32(strmessage.c_str());//after ntohl 
	if (kMinMessageLen < len && len < kMaxMessageLen)
	{
		if (len == strmessage.size() - kHeaderLen - 2)//checksum and "\r\n"
		{
			shared_ptr_Msg message = Decode(strmessage.c_str() + kHeaderLen, len);
			//handle message callback
			messageCallback_(connection, message);
		}
		else
		{
			//invaild message
		}
	}
	else
	{
		//Invalid length
		//errorCallback_
	}
}
void ProtobufCodec::onSyncMessage(const std::string& strmessage)
{
	//message length
	const uint32_t len = PeekUint32(strmessage.c_str());//after ntohl 
	if (kMinMessageLen < len && len < kMaxMessageLen)
	{
		if (len == strmessage.size() - kHeaderLen - 2)//checksum and "\r\n"
		{
			shared_ptr_Msg message = Decode(strmessage.c_str() + kHeaderLen, len);
			//handle message callback
			syncMessageCallback_(message);
		}
		else
		{
			//invaild message
		}
	}
	else
	{
		//Invalid length
		//errorCallback_
	}
}
void ProtobufCodec::Send(const TcpConnectionPtr& connection, const google::protobuf::Message& message)
{
	// FIXME: serialize to TcpConnection::outputBuffer()
	m_mutex.lock();
	auto buf = Encode(message);
	m_mutex.unlock();
	connection->Send(buf);

}