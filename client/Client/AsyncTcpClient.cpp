#include "stdafx.h"
#include "AsyncTcpClient.h"
#include <stdint.h>

// #if !defined(UNDER_CE) && defined(_DEBUG)
// #define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
// #endif

AsyncTcpClient::AsyncTcpClient(boost::asio::io_service& service,  CWnd* ptrWnd)
: m_service(service)
, m_socket(service)
, m_ptrWnd(ptrWnd)
, m_isConnected(false)
, m_delimiter("\r\n")
{
	
}


AsyncTcpClient::~AsyncTcpClient()
{
	Disconnect();
	google::protobuf::ShutdownProtobufLibrary();
}

void AsyncTcpClient::Connect(const std::string& address, uint16_t port)
{
	try
	{
		boost::asio::ip::tcp::endpoint m_endpoint(
			boost::asio::ip::address::from_string(address), port);
		Connect(m_endpoint);
	}
	catch (CException* e)
	{

	}

}
void AsyncTcpClient::Connect(const std::string &url, const std::string &protocol)
{	
	boost::asio::ip::tcp::resolver::query query(url, protocol);
	boost::asio::ip::tcp::resolver resolver(m_service);
	boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

	boost::asio::async_connect(m_socket, it
		, boost::bind(&AsyncTcpClient::handle_connect, this, boost::asio::placeholders::error));
}
void AsyncTcpClient::Connect(const boost::asio::ip::tcp::endpoint& endpoint)
{
	if (m_isConnected) return;
	m_socket.async_connect(endpoint
 		, boost::bind(&AsyncTcpClient::handle_connect, this, boost::asio::placeholders::error));
}
void AsyncTcpClient::Disconnect()
{
	if (m_socket.is_open())
	{
		m_socket.close();
	}
	// tell the IO service to stop
	m_socket.get_io_service().stop();

	m_isConnected = false;
}
void AsyncTcpClient::Reconnect(const boost::system::error_code& error)
{
	if (m_isConnected) return;
	// close current socket if necessary
	if (m_socket.is_open())
		m_socket.close();

	m_socket.async_connect(m_endPoint
		, boost::bind(&AsyncTcpClient::handle_connect, this, boost::asio::placeholders::error));
}
//connect callback
void AsyncTcpClient::handle_connect(const boost::system::error_code& error)
{
	if (!error)
	{
		//we are connected
		m_isConnected = true;
		//get remote address
		m_endPoint = m_socket.remote_endpoint();
		// wait for a message to arrive, then call handle_read
		read();
	}
	else
	{
		// there was an error :(
		m_isConnected = false;
	}
}
void AsyncTcpClient::read()
{
	boost::asio::async_read_until(m_socket
		, m_buffer
		, m_delimiter
		, boost::bind(&AsyncTcpClient::handle_read
		, this
		, boost::asio::placeholders::error
		, boost::asio::placeholders::bytes_transferred));
}
void AsyncTcpClient::handle_read(const boost::system::error_code& e, uint32_t bytes_transferred)
{
	if (!e)
	{
		// TODO: you could do some message processing here, like breaking it up
		// into smaller parts, rejecting unknown messages or handling the message protocol
		std::string message(boost::asio::buffer_cast<const char*>(m_buffer.data()), bytes_transferred);

		m_buffer.consume(bytes_transferred);
		// create function to notify listeners
		messageCallback_(shared_from_this(), message);

		// wait for the next message
		read();
	}
	else
	{
		m_socket.close();
	}
}
void AsyncTcpClient::handle_write(const boost::system::error_code& e, uint32_t bytes_transferred)
{
	if (!e)
	{
		//删除已经发送的数据
		if (!m_sendMessages.empty())
		{
			assert(bytes_transferred == m_sendMessages.front()->readableBytes());
			m_sendMessages.front()->consume(bytes_transferred);
			m_sendMessages.pop();
// 			if (!m_sendMessages.empty())
// 			{
// 				m_socket.async_send(boost::asio::buffer(m_sendMessages.front()->peek(), m_sendMessages.front()->readableBytes()),
// 					boost::bind(&AsyncTcpClient::handle_write, this,
// 					boost::asio::placeholders::error,
// 					boost::asio::placeholders::bytes_transferred));
// 			}
		}
	}
	else
	{
		m_socket.close();
	}
}

void AsyncTcpClient::Send(std::shared_ptr<MyBuffer> ptrBuffer)
{
	if (m_isConnected)
	{
		m_sendMessages.push(ptrBuffer);
		// safe way to request the client to write a message
		// send all date
		//rdbuf Manages the associated stream buffer.
		m_socket.async_send(boost::asio::buffer(ptrBuffer->peek(), ptrBuffer->readableBytes()),
			boost::bind(&AsyncTcpClient::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
}


