#ifndef PROTOBUFCODEC_H
#define PROTOBUFCODEC_H

#include <memory>
#include <zlib/zlib.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <functional>
#include "MyBuffer.h"
#include "Tcp_Session.h"
#include <assert.h>
#pragma comment(lib,"zlib.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libprotoc.lib")
// The wire format of an protocol header is:
// +--------------------------------------------------------------+ 
// |                          all length                          |     uint32
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
typedef std::shared_ptr<google::protobuf::Message>  shared_ptr_Msg;
typedef google::protobuf::Message CMessage;
typedef google::protobuf::Message* ptrMessage;
class ProtobufCodec
{
public:
	const static int kHeaderLen = sizeof(int32_t);
	const static int kChecksumLen = sizeof(int32_t);
	const static int kMinMessageLen = 2 * kHeaderLen + 2; // nameLen + typeName + checkSum
	const static int kMaxMessageLen = 64 * 1024 * 1024; // same as codec_stream.h kDefaultTotalBytesLimit

	typedef std::function<void(const SessionPtr&,
		const shared_ptr_Msg&)> ProtobufMessageCallback;

	explicit ProtobufCodec(const ProtobufMessageCallback& callback);

	void Send(const SessionPtr& connection,
		const google::protobuf::Message& message)
	{
		// FIXME: serialize to TcpConnection::outputBuffer()
		auto buf = Encode(message);
		connection->Send(buf);
	}
	~ProtobufCodec();
	void onMessage(const SessionPtr& connection, const std::string& strmessage);
	//消息基类
	//create message object by typename
	static shared_ptr_Msg CreateMessage(const std::string& type_name);
	//加密消息;
	static std::shared_ptr<MyBuffer> Encode(const CMessage& message);
	//create message object from buffer
	static shared_ptr_Msg Decode(const char* buf, uint32_t len);
private:
	//检测消息是否正确
	// +--------------------------------------------------------------+     
	// |                       message name length                    |     uint32
	// +--------------------------------------------------------------+
	// |                                                              |
	// |                          message  name                       |
	// |                                                              |
	// +--------------------------------------------------------------+
	// |                                                              |
	// |                             data                             |
	// |                                                              |
	// +--------------------------------------------------------------+ 
	//check sum
	inline static bool IsCorrect(const char* buf, int32_t len);
	//从字符串中读取数字
	inline static uint32_t PeekUInt32(const char* buf);

private:
	ProtobufMessageCallback messageCallback_;
};

#endif	//PROTOBUFCODEC_H