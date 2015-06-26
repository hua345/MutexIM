#include "stdafx.h"
#include "UDP_Server.h"

//constructor
UDP_Server::UDP_Server(boost::asio::io_service& io)
:m_socket(io, boost::asio::ip::udp::endpoint(
boost::asio::ip::udp::v4(), 6000))
{
	Start_receive();
}

//destructor
UDP_Server::~UDP_Server()
{
}
//just test
std::string UDP_Server::make_daytime_string()
{
	time_t now = time(0);
	return ctime(&now);
}

void UDP_Server::Start_receive()
{
	//async receive
	m_socket.async_receive_from(
		boost::asio::buffer(m_recvBuf), m_remote_endpoint
		, boost::bind(&UDP_Server::Handle_receive, this
		, boost::asio::placeholders::error
		, boost::asio::placeholders::bytes_transferred));
}
//receive callback handle
void UDP_Server::Handle_receive(const boost::system::error_code &e, size_t bytes_transferred)
{
	if (boost::asio::error::eof == e)
	{
		std::cout << "Client closed" << std::endl;
		return;
	}
	else if (!e || e == boost::asio::error::message_size)
	{
		std::cout << m_recvBuf << std::endl;
		//just for test ,return current time
		auto m_message = std::make_shared<std::string>(make_daytime_string());
		//async send data
		m_socket.async_send_to(boost::asio::buffer(*m_message)
			, m_remote_endpoint
			, boost::bind(&UDP_Server::Handle_send, this
			, boost::asio::placeholders::error
			, boost::asio::placeholders::bytes_transferred));

		Start_receive();
	}
}
//send callback handle
void UDP_Server::Handle_send(const boost::system::error_code &e, size_t bytes_transferred)
{
	//no-thing
}
