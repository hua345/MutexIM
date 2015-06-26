
#include "stdafx.h"

#include "tcp_server.h"
//tcp constructor
tcp_server::tcp_server(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint)
: acceptor_(io_service, endpoint)
, connection_manager_()
, new_connection_()
{
	start_accept();
}

tcp_server::~tcp_server()
{
	connection_manager_.stop_all();
}
void tcp_server::start_accept()
{
	//重新设置指向对话实例的指针
	new_connection_.reset(new Tcp_Session(acceptor_.get_io_service()));

	acceptor_.async_accept(new_connection_->socket()
		, boost::bind(&tcp_server::handle_accept
		, this
		, boost::asio::placeholders::error));
}
void tcp_server::handle_accept(const boost::system::error_code& error)
{
	// Check whether the server was stopped by a signal before this completion
	// handler had a chance to run.
	if (!acceptor_.is_open())
	{
		return;
	}
	if (!error)
	{
		//register  callback
		new_connection_->setConnectionCallback(connectionCallback_);
		new_connection_->setMessageCallback(messageCallback_);
		new_connection_->setWriteCompleteCallback(writeCompleteCallback_);
		new_connection_->setDisconnectedCallback(
			boost::bind(&tcp_server::removeConnection, this, _1));
		connection_manager_.start(new_connection_);
	}
	//accept new client connection
	start_accept();
}

void tcp_server::handle_stop()
{
	// The server is stopped by cancelling all outstanding asynchronous
	// operations. Once all operations have finished the io_service::run() call
	// will exit.
	acceptor_.close();
	connection_manager_.stop_all();
}

void tcp_server::removeConnection(const SessionPtr& session)
{
	connection_manager_.stop(session);
}

