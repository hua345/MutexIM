#ifndef DEMO_TCP_SESSION
#define DEMO_TCP_SESSION

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory.h>
#include <iostream>
#include <stdint.h>
#include <queue>
#include <boost/asio/streambuf.hpp>
#include "MyBuffer.h"
#include <iostream>
using namespace boost::asio::ip;

//We will use shared_ptr and enable_shared_from_this because we want to 
//keep the tcp_connection object alive as long as there is an operation that refers to it.
class Tcp_Session;

typedef std::shared_ptr<Tcp_Session> SessionPtr;
typedef std::function<void(const SessionPtr&)> ConnectedCallback;
typedef std::function<void(const SessionPtr&)> DisconnectedCallback;

typedef std::function<void(const SessionPtr&, const std::string&)> MessageCallback;//when have message
typedef std::function<void(const SessionPtr&)> WriteCompleteCallback;
class Tcp_Session
	: public std::enable_shared_from_this<Tcp_Session>
{
public:
	typedef boost::asio::streambuf  StreamBuf;

	//return current socket refference
	tcp::socket& socket()
	{
		return m_socket;
	}
	//private constructor
	explicit Tcp_Session(boost::asio::io_service& io_service);
	~Tcp_Session();
	/// Start the first asynchronous operation for the connection.
	void start();
	/// Stop all asynchronous operations associated with the connection.
	void stop();
	/**
	* @brief Send message to server
	* @param[in] msg Want to send a message.
	*/
	void Send(std::shared_ptr<MyBuffer> ptrBuffer);

	//set message callback
	void setMessageCallback(const MessageCallback& callback)
	{
		messageCallback_ = callback;
	}
	void setConnectionCallback(const ConnectedCallback& callback)
	{
		ConnectedCallback_ = callback;
	}
	void setDisconnectedCallback(const DisconnectedCallback& callback)
	{
		DisconnectedCallback_ = callback;
	}
	void setWriteCompleteCallback(const WriteCompleteCallback& callback)
	{
		writeCompleteCallback_ = callback;
	}
	void set_SessionId(const std::string& sessionId)
	{
		m_sessionID = sessionId;
	}
	const std::string& get_SessionId()
	{
		return m_sessionID;
	}

	void set_userId(const std::string& userid)
	{
		m_userId = userid;
	}

	const std::string& get_userId()
	{
		return m_userId;
	}
private:
	//read && write
	void read();
	/// Handle completion of a read operation.
	void handle_read(const boost::system::error_code& error
		, uint32_t byte_transferred);
	/// Handle completion of a write operation.
	void handle_write(const boost::system::error_code& error,
		uint32_t bytes_transferred);
	void close();
	void do_close();
private:
	/// Socket for the connection.
	boost::asio::ip::tcp::socket m_socket;

	/// The handler used to process the incoming request.
	MessageCallback messageCallback_;			
	ConnectedCallback ConnectedCallback_;		
	DisconnectedCallback DisconnectedCallback_; 
	WriteCompleteCallback writeCompleteCallback_;

	std::string m_delimiter;
private:
	//connected state
	bool m_isConnected;//判断是否连接
	//recvive buffer
	StreamBuf m_buffer;
	//sessionID
	std::string m_sessionID;
	//消息队列
	std::queue<std::shared_ptr<MyBuffer>> m_sendMessages;

	std::string m_userId;//用户ID;
};

#endif