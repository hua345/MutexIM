#include "stdafx.h"
#include "MyBuffer.h"
//·Ö¸î·û
const char MyBuffer::kCRLF[] = "\r\n";

MyBuffer::MyBuffer(size_t initialSize /*= kInitialSize*/)
: buffer_(kCheapPrepend + initialSize),
readerIndex_(kCheapPrepend),
writerIndex_(kCheapPrepend)
{
	assert(readableBytes() == 0);
	assert(writableBytes() == initialSize);
	assert(prependableBytes() == kCheapPrepend);
}


MyBuffer::~MyBuffer()
{

}

void MyBuffer::makeSpace(size_t len)
{
	if (writableBytes() + prependableBytes() < len + kCheapPrepend)
	{
		// move readable data,realloc memory
		buffer_.resize(writerIndex_ + len);
	}
	else
	{
		// move readable data to the front, make space inside buffer
		assert(kCheapPrepend < readerIndex_);
		size_t readable = readableBytes();
		//move data
		//memcpy(begin() + kCheapPrepend, begin() + readerIndex_, readableBytes());
		std::copy(begin() + readerIndex_,
			begin() + writerIndex_,
			buffer_.begin() + kCheapPrepend);
		readerIndex_ = kCheapPrepend;
		writerIndex_ = readerIndex_ + readable;
		//set '\0' for test
		memset(begin() + writerIndex_, '\0', readable);
		assert(readable == readableBytes());
	}
}
