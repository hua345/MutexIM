#ifndef DEMO_TCP_SERVER
#define DEMO_TCP_SERVER

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/asio.hpp>
#include <memory>
#include <boost/noncopyable.hpp>
#include "ssl_Session.h"
#include "Connection_Manager.h"


#pragma comment(lib, "libeay32.lib")  
#pragma comment(lib, "ssleay32.lib") 
class ssl_server:private boost::noncopyable
{
public:
	//constructor
	explicit ssl_server(boost::asio::io_service& io_service,const boost::asio::ip::tcp::endpoint& endpoint);
	~ssl_server();
	std::string get_password() const;
public:
	void setConnectedCallback(const TConnectedCallback& callback)
	{
		connectionCallback_ = callback;
	}
	void setMessageCallback(const TMessageCallback& callback)
	{
		messageCallback_ = callback;
	}
	void setWriteCompleteCallback(const TWriteCompleteCallback& callback)
	{
		writeCompleteCallback_ = callback;
	}
private:
	TConnectedCallback connectionCallback_;
	TMessageCallback messageCallback_;
	TWriteCompleteCallback writeCompleteCallback_;
private:
	/// Initiate an asynchronous accept operation.
	void start_accept();
	
	/// Handle completion of an asynchronous accept operation.
	void handle_accept(const boost::system::error_code& error);

	/// Handle a request to stop the server.
	void handle_stop();
	/// Acceptor used to listen for incoming connections.
	void removeConnection(const TSessionPtr& session);
	boost::asio::ip::tcp::acceptor acceptor_;
	//SSL context
	boost::asio::ssl::context context_;
	/// The connection manager which owns all live connections.
	Connection_Manager connection_manager_;

	/// The next connection to be accepted.
	TSessionPtr new_connection_;

};

#endif	//DEMO_TCP_SERVER