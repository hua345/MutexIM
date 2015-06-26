
#include "stdafx.h"

#include "ssl_server.h"
//tcp constructor
ssl_server::ssl_server(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint)
: acceptor_(io_service, endpoint)
, context_(boost::asio::ssl::context::sslv23)
, connection_manager_()
, new_connection_()
{
	//SSL Setting
	context_.set_options(
		boost::asio::ssl::context::default_workarounds
		| boost::asio::ssl::context::no_sslv2
		| boost::asio::ssl::context::single_dh_use);
	context_.set_verify_mode(boost::asio::ssl::context::verify_fail_if_no_peer_cert);

	context_.set_password_callback(std::bind(&ssl_server::get_password, this));
	//context_.load_verify_file("server.csr");
	context_.use_certificate_chain_file("server.crt");
	context_.use_private_key_file("server.key", boost::asio::ssl::context::pem);
	context_.use_tmp_dh_file("dh1024.pem");


	start_accept();
}

ssl_server::~ssl_server()
{
	connection_manager_.stop_all();
}
std::string ssl_server::get_password() const
{
	return "server_encrypto";
}
void ssl_server::start_accept()
{
	//重新设置指向对话实例的指针
	new_connection_.reset(new ssl_Session(acceptor_.get_io_service(), context_));

	acceptor_.async_accept(new_connection_->socket()
		, boost::bind(&ssl_server::handle_accept
		, this
		, boost::asio::placeholders::error));
}
void ssl_server::handle_accept(const boost::system::error_code& error)
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
			boost::bind(&ssl_server::removeConnection, this, _1));
		connection_manager_.start(new_connection_);
	}
	//accept new client connection
	start_accept();
}

void ssl_server::handle_stop()
{
	// The server is stopped by cancelling all outstanding asynchronous
	// operations. Once all operations have finished the io_service::run() call
	// will exit.
	acceptor_.close();
	connection_manager_.stop_all();
}

void ssl_server::removeConnection(const TSessionPtr& session)
{
	connection_manager_.stop(session);
}

