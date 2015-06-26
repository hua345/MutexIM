#ifndef DEMO_TCP_SERVER
#define DEMO_TCP_SERVER

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/asio.hpp>
#include <memory>
#include <boost/noncopyable.hpp>
#include "Tcp_Session.h"
#include "Connection_Manager.h"

class tcp_server:private boost::noncopyable
{
public:
	//constructor
	explicit tcp_server(boost::asio::io_service& io_service,const boost::asio::ip::tcp::endpoint& endpoint);
	~tcp_server();
public:
	void setConnectedCallback(const ConnectedCallback& callback)
	{
		connectionCallback_ = callback;
	}
	void setMessageCallback(const MessageCallback& callback)
	{
		messageCallback_ = callback;
	}
	void setWriteCompleteCallback(const WriteCompleteCallback& callback)
	{
		writeCompleteCallback_ = callback;
	}
	Connection_Manager& getConnection_Manager(){ return connection_manager_; }
private:
	ConnectedCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
private:
	/// Initiate an asynchronous accept operation.
	void start_accept();
	
	/// Handle completion of an asynchronous accept operation.
	void handle_accept(const boost::system::error_code& error);

	/// Handle a request to stop the server.
	void handle_stop();
	/// Acceptor used to listen for incoming connections.
	void removeConnection(const SessionPtr& session);
	boost::asio::ip::tcp::acceptor acceptor_;

	/// The connection manager which owns all live connections.
	Connection_Manager connection_manager_;

	/// The next connection to be accepted.
	SessionPtr new_connection_;
};

#endif	//DEMO_TCP_SERVER