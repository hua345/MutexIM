#ifndef DEMO_TCP_SESSION
#define DEMO_TCP_SESSION

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
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
class ssl_Session;

typedef std::shared_ptr<ssl_Session> TSessionPtr;
typedef std::function<void(const TSessionPtr&)> TConnectedCallback;
typedef std::function<void(const TSessionPtr&)> TDisconnectedCallback;

typedef std::function<void(const TSessionPtr&, const std::string&)> TMessageCallback;//when have message
typedef std::function<void(const TSessionPtr&)> TWriteCompleteCallback;
class ssl_Session
	: public std::enable_shared_from_this<ssl_Session>
{
public:
	typedef boost::asio::streambuf  TStreambuf;
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> TSecureSocket;
	//return current socket refference
	TSecureSocket::lowest_layer_type& socket()
	{
		return m_sslSocket.lowest_layer();
	}
	//private constructor
	explicit ssl_Session(boost::asio::io_service& io_service, boost::asio::ssl::context& sslContext);
	~ssl_Session();
	/// Start the first asynchronous operation for the connection.
	void start();
	/// Stop all asynchronous operations associated with the connection.
	void stop();
	/**
	* @brief Send message to server
	* @param[in] msg Want to send a message.
	*/
	void sslSend(std::shared_ptr<MyBuffer> ptrBuffer);
	//set message callback
	void setMessageCallback(const TMessageCallback& callback)
	{
		messageCallback_ = callback;
	}
	void setConnectionCallback(const TConnectedCallback& callback)
	{
		ConnectedCallback_ = callback;
	}
	void setDisconnectedCallback(const TDisconnectedCallback& callback)
	{
		DisconnectedCallback_ = callback;
	}
	void setWriteCompleteCallback(const TWriteCompleteCallback& callback)
	{
		writeCompleteCallback_ = callback;
	}
private:
	//read && write
	void read();
	/// Handle completion of a read operation.
	void handle_read(const boost::system::error_code& error, uint32_t byte_transferred);
	/// Handle completion of a write operation.
	void handle_write(const boost::system::error_code& error, uint32_t bytes_transferred);
	//SSL handshake
	void handle_handshake(const boost::system::error_code& error);

private:
	/// SSL socket 
	TSecureSocket m_sslSocket;

	/// The handler used to process the incoming request.
	TMessageCallback messageCallback_;			
	TConnectedCallback ConnectedCallback_;		
	TDisconnectedCallback DisconnectedCallback_; 
	TWriteCompleteCallback writeCompleteCallback_;

	std::string m_delimiter;
private:
	//connected state
	bool m_isConnected;//判断是否连接
	//recvive buffer
	TStreambuf m_buffer;
	//消息队列
	std::queue<std::shared_ptr<MyBuffer>> m_sendMessages;
};

#endif