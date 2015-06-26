#pragma once
#ifndef SYNCPROTOBUFCODEC_H
#define SYNCPROTOBUFCODEC_H

#include <memory>
#include <zlib/zlib.h>			//for adler32
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <functional>
#include <stdint.h>
#include "MyBuffer.h"
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
class SyncProtobufCodec
{
public:
	const static int kHeaderLen = sizeof(int32_t);
	const static int kChecksumLen = sizeof(int32_t);
	const static int kMinMessageLen = 2 * kHeaderLen + 2; // nameLen + typeName + checkSum
	const static int kMaxMessageLen = 64 * 1024 * 1024; // same as codec_stream.h kDefaultTotalBytesLimit

	typedef std::function<void(const shared_ptr_Msg&)> SyncMessageCallback;	//sync connection callback
	explicit SyncProtobufCodec(const SyncMessageCallback& callback);

	~SyncProtobufCodec();
	void onSyncMessage(const std::string& strmessage);
	//消息基类
	//create message object by typename
	static shared_ptr_Msg CreateMessage(const std::string& type_name);
	//
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
	inline static uint32_t PeekUint32(const char* buf);

private:
	SyncMessageCallback     syncMessageCallback_;
};

#endif	//SYNCPROTOBUFCODEC_H

