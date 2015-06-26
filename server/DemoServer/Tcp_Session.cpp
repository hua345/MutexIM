
#include "stdafx.h"

#include "Tcp_Session.h"
//private constructor
Tcp_Session::Tcp_Session(boost::asio::io_service& io_service)
: m_socket(io_service)
, m_isConnected(false)
, m_delimiter("\r\n")
{

}
//free resource
Tcp_Session::~Tcp_Session()
{
	stop();
}
//connection start function
void Tcp_Session::start()
{
// 	//first connection return flags
// 	m_socket.async_write_some(boost::asio::buffer("link successed!")
// 		, boost::bind(&Tcp_Session::handle_write, shared_from_this()
// 		, boost::asio::placeholders::error
// 		, boost::asio::placeholders::bytes_transferred));

	m_isConnected = true;
	//get client IP address
	std::cout << "get new client  " << m_socket.remote_endpoint().address() << std::endl;
	read();
}
void Tcp_Session::stop()
{
	if (m_socket.is_open())
	{
		m_socket.close();
	}
	m_isConnected = false;
}
void Tcp_Session::read()
{
	boost::asio::async_read_until(m_socket
		, m_buffer
		, m_delimiter
		, boost::bind(&Tcp_Session::handle_read
		, this
		, boost::asio::placeholders::error
		, boost::asio::placeholders::bytes_transferred));
}
void Tcp_Session::handle_read(const boost::system::error_code& e, uint32_t bytes_transferred)
{
	if (!e)
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
	else if (e != boost::asio::error::operation_aborted)
	{
		DisconnectedCallback_(shared_from_this());
	}
}
void Tcp_Session::handle_write(const boost::system::error_code& e, uint32_t bytes_transferred)
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
// 					boost::bind(&Tcp_Session::handle_write, this,
// 					boost::asio::placeholders::error,
// 					boost::asio::placeholders::bytes_transferred));
// 			}
		}
	}
	else
	{
		DisconnectedCallback_(shared_from_this());
	}
}

void Tcp_Session::Send(std::shared_ptr<MyBuffer> ptrBuffer)
{
	if (m_isConnected)
	{
		m_sendMessages.push(ptrBuffer);
		// safe way to request the client to write a message
		// send all date
		//rdbuf Manages the associated stream buffer.
		m_socket.async_send(boost::asio::buffer(ptrBuffer->peek(), ptrBuffer->readableBytes()),
			boost::bind(&Tcp_Session::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
}
