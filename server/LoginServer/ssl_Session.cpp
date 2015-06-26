
#include "stdafx.h"

#include "ssl_Session.h"
//private constructor
ssl_Session::ssl_Session(boost::asio::io_service& io_service, boost::asio::ssl::context& sslContext)
: m_sslSocket(io_service, sslContext)
, m_isConnected(false)
, m_delimiter("\r\n")
{

}
//free resource
ssl_Session::~ssl_Session()
{
	stop();
}
//connection start function
void ssl_Session::start()
{
// 	//first connection return flags
// 	m_socket.async_write_some(boost::asio::buffer("link successed!")
// 		, boost::bind(&Tcp_Session::handle_write, shared_from_this()
// 		, boost::asio::placeholders::error
// 		, boost::asio::placeholders::bytes_transferred));

	m_isConnected = true;
	//get client IP address
	std::cout << "get new client  " << m_sslSocket.lowest_layer().remote_endpoint().address() << std::endl;
	//SSL handeshake
	m_sslSocket.async_handshake(boost::asio::ssl::stream_base::server,
		boost::bind(&ssl_Session::handle_handshake, this,
		boost::asio::placeholders::error));
}
void ssl_Session::stop()
{
	try {
		if (m_sslSocket.next_layer().is_open()) {
			m_sslSocket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			m_sslSocket.next_layer().close();
		}
	}
	catch (boost::system::error_code& err) {
		/**< TODO:记录错误信息 */

	}
	
	m_isConnected = false;
}
void ssl_Session::read()
{
	boost::asio::async_read_until(m_sslSocket
		, m_buffer
		, m_delimiter
		, boost::bind(&ssl_Session::handle_read
		, this
		, boost::asio::placeholders::error
		, boost::asio::placeholders::bytes_transferred));
}
void ssl_Session::handle_read(const boost::system::error_code& err, uint32_t bytes_transferred)
{
	if (!err)
	{
		// TODO: you could do some message processing here, like breaking it up
		// into smaller parts, rejecting unknown messages or handling the message protocol
		std::string message(boost::asio::buffer_cast<const char*>(m_buffer.data()), bytes_transferred);

		//std::cout <<"Tcp_Session::handle_read : "<< message << std::endl;
		m_buffer.consume(bytes_transferred);
		// create function to notify listeners
		messageCallback_(shared_from_this(), message);

		// wait for the next message
		read();
	}
	else if (err != boost::asio::error::operation_aborted)
	{
		DisconnectedCallback_(shared_from_this());
	}
}
void ssl_Session::handle_write(const boost::system::error_code& err, uint32_t bytes_transferred)
{
	if (!err)
	{
		//删除已经发送的数据
		if (!m_sendMessages.empty())
		{
			assert(bytes_transferred == m_sendMessages.front()->readableBytes());
			m_sendMessages.front()->consume(bytes_transferred);
			m_sendMessages.pop();
// 			if (!m_sendMessages.empty())
// 			{
// 				
// 				boost::asio::async_write(m_sslSocket
// 					, boost::asio::buffer(m_sendMessages.front()->peek(), m_sendMessages.front()->readableBytes())
// 					, boost::bind(&ssl_Session::handle_write
// 					, this
// 					, boost::asio::placeholders::error
// 					, boost::asio::placeholders::bytes_transferred));
// 			}
		}
	}
	else
	{
		DisconnectedCallback_(shared_from_this());
	}
}

void ssl_Session::sslSend(std::shared_ptr<MyBuffer> ptrBuffer)
{
	if (m_isConnected)
	{
		m_sendMessages.push(ptrBuffer);
		// safe way to request the client to write a message
		// send all date
		//rdbuf Manages the associated stream buffer.
		boost::asio::async_write(m_sslSocket
			, boost::asio::buffer(ptrBuffer->peek(), ptrBuffer->readableBytes())
			, boost::bind(&ssl_Session::handle_write
			, this
			, boost::asio::placeholders::error
			, boost::asio::placeholders::bytes_transferred));
	}
}
void ssl_Session::handle_handshake(const boost::system::error_code& error)
{
	if (!error)
	{
		//begin read data
		read();
	}
	else
	{
		DisconnectedCallback_(shared_from_this());
	}
}
