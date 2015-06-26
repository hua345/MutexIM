#ifndef MYBUFFER_H
#define MYBUFFER_H

/// 
/// +-------------------+------------------+------------------+
/// | prependable bytes | readable bytes   | writable bytes   |
/// |                   |    (CONTENT)     |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0        <=     readerIndex <=   writerIndex    <=       size
///
#pragma once
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <boost/detail/endian.hpp>
#ifdef WIN32
//window
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#elif define __linux_
//linux
#include <arpa/inet.h>
#elif define __unix__
//unix
#endif

class MyBuffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;
	static const size_t kmaxSize = 10 * 1024;
	explicit MyBuffer(size_t initialSize = kInitialSize);
	~MyBuffer();
	//*************************************************************************************************//
	//read position
	inline const char* peek() const
	{
		return begin() + readerIndex_;
	}
	const char* findCRLF() const
	{
		// FIXME: replace with memmem()?
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}
	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		// FIXME: replace with memmem()?
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}
	const char* findEOL() const
	{
		const void* eol = memchr(peek(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	}
	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}
	//begin write position
	char* beginWrite()
	{
		return begin() + writerIndex_;
	}
	const char* beginWrite() const
	{
		return begin() + writerIndex_;
	}
	//prepend data
	void prepend(const void* /*restrict*/ data, size_t len)
	{
		assert(len <= prependableBytes());
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		//memcpy(begin() + readerIndex_, data, len);
		std::copy(d, d + len, buffer_.begin() + readerIndex_);
	}
	///
	/// Prepend int32_t using network endian
	///
	void prependInt32(int32_t x)
	{
		int32_t be32 = htonl(x);
		prepend(&be32, sizeof(be32));
	}
	void prependInt16(int16_t x)
	{
		int16_t be16 = htons(x);
		prepend(&be16, sizeof be16);
	}
	//
	//append data
	//
	void append(const std::string& str)
	{
		append(str.data(), str.size());
	}
	//append data
	void append(const char* /*restrict*/ data, size_t len)
	{
		ensureWritableBytes(len);
		//memcpy(beginWrite(), data, len);
		std::copy(data, data + len, buffer_.begin() + writerIndex_);
		commit(len);
	}
	//append data
	void append(const void* /*restrict*/ data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}
	///
	/// Append int32_t using network endian
	///
	void appendUint32(uint32_t x)
	{
		uint32_t be32 = htonl(x);
		append(&be32, sizeof(be32));
	}
	///
	/// Append int16_t using network endian
	///
	void appendInt16(int16_t x)
	{
		int16_t be16 = htons(x);
		append(&be16, sizeof(be16));
	}
	///
	/// Read int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	uint32_t readUint32()
	{
		uint32_t result = peekUint32();
		consume(sizeof(result));
		return result;
	}
// 	int16_t readInt16()
// 	{
// 		int16_t result = peekInt16();
// 		consume(sizeof(result));
// 		return result;
// 	}
	///
	/// Peek int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	uint32_t peekUint32() const
	{
		assert(readableBytes() >= sizeof(uint32_t));
		uint32_t be32 = 0;
		memcpy(&be32, peek(), sizeof(be32));
		return ntohl(be32);
	}
// 	int16_t peekInt16() const
// 	{
// 		assert(readableBytes() >= sizeof(int16_t));
// 		int16_t be16 = 0;
// 		memcpy(&be16, peek(), sizeof be16);
// 		return ntohs(be16);
// 	}
	/// Move characters from the output sequence to the input sequence.
	void commit(size_t len)
	{
		assert(len <= writableBytes());
		writerIndex_ += len;
	}
	//Removes n characters from the beginning of the input sequence.
	void consume(size_t len)
	{
		assert(len <= readableBytes());
		if (len < readableBytes())
		{
			//set '\0' for test
			memset(begin() + readerIndex_, '\0', len);
			readerIndex_ += len;
		}
		else
		{
			//set '\0' for test
			memset(begin() + readerIndex_, '\0', len);
			consumeAll();
		}
	}
	//reset index
	inline void consumeAll(){
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
	}

	//ensure memory enough
	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}
	//readable size
	inline  size_t readableBytes() const
	{
		return writerIndex_ - readerIndex_;
	}
	//writeable size
	inline  size_t writableBytes() const
	{
		return buffer_.size() - writerIndex_;
	}
	//检测内存是否足够写 len 大小数据
	void makeSpace(size_t len);
private:


	//prepend size
	inline size_t prependableBytes() const
	{
		return readerIndex_;
	}
	inline char* begin()
	{
		return &*buffer_.begin();
	}
	inline const char* begin() const
	{
		return &*buffer_.begin();
	}


private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
	static const char kCRLF[];  //分隔符 
};

#endif